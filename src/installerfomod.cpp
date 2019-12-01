#include "installerfomod.h"

#include "filenamestring.h"
#include "fomodinstallerdialog.h"
#include "imodinterface.h"
#include "imodlist.h"

#include <report.h>
#include <iinstallationmanager.h>
#include <utility.h>

#include <QtPlugin>
#include <QStringList>
#include <QImageReader>
#include <QDebug>


using namespace MOBase;


InstallerFomod::InstallerFomod()
  : m_MOInfo(nullptr)
{
}

bool InstallerFomod::init(IOrganizer *moInfo)
{
  m_MOInfo = moInfo;
  return true;
}

QString InstallerFomod::name() const
{
  return "Fomod Installer";
}

QString InstallerFomod::author() const
{
  return "Tannin & thosrtanner";
}

QString InstallerFomod::description() const
{
  return tr("Installer for xml based fomod archives.");
}

VersionInfo InstallerFomod::version() const
{
  return VersionInfo(1, 6, 1, VersionInfo::RELEASE_FINAL);
}

bool InstallerFomod::isActive() const
{
  return m_MOInfo->pluginSetting(name(), "enabled").toBool();
}

bool InstallerFomod::allowAnyFile() const
{
  return m_MOInfo->pluginSetting(name(), "use_any_file").toBool();
}

bool InstallerFomod::checkDisabledMods() const
{
  return m_MOInfo->pluginSetting(name(), "see_disabled_mods").toBool();
}

QList<PluginSetting> InstallerFomod::settings() const
{
  QList<PluginSetting> result;
  result.push_back(PluginSetting("enabled", "check to enable this plugin", QVariant(true)));
  result.push_back(PluginSetting("prefer", "prefer this over the NCC based plugin", QVariant(true)));
  result.push_back(PluginSetting("use_any_file", "allow dependencies on any file, not just esp/esm", QVariant(false)));
  result.push_back(PluginSetting("see_disabled_mods", "treat disabled mods as inactive rather than missing", QVariant(false)));
  return result;
}

unsigned int InstallerFomod::priority() const
{
  return m_MOInfo->pluginSetting(name(), "prefer").toBool() ? 110 : 90;
}


bool InstallerFomod::isManualInstaller() const
{
  return false;
}


const DirectoryTree *InstallerFomod::findFomodDirectory(const DirectoryTree *tree) const
{
  for (DirectoryTree::const_node_iterator iter = tree->nodesBegin();
       iter != tree->nodesEnd(); ++iter) {
    const FileNameString &dirName = (*iter)->getData().name;
    if (dirName == "fomod") {
      return *iter;
    }
  }
  if ((tree->numNodes() == 1) && (tree->numLeafs() == 0)) {
    return findFomodDirectory(*tree->nodesBegin());
  }
  return nullptr;
}


bool InstallerFomod::isArchiveSupported(const DirectoryTree &tree) const
{
  const DirectoryTree *fomodDir = findFomodDirectory(&tree);
  if (fomodDir != nullptr) {
    for (DirectoryTree::const_leaf_iterator fileIter = fomodDir->leafsBegin();
         fileIter != fomodDir->leafsEnd(); ++fileIter) {
      if (fileIter->getName() == "ModuleConfig.xml") {
        return true;
      }
    }
  }
  return false;
}

void InstallerFomod::appendImageFiles(QStringList &result, DirectoryTree *tree)
{
  for (auto iter = tree->leafsBegin(); iter != tree->leafsEnd(); ++iter) {
    if ((iter->getName().endsWith(".png")) ||
        (iter->getName().endsWith(".jpg")) ||
        (iter->getName().endsWith(".gif")) ||
        (iter->getName().endsWith(".bmp"))) {
      result.append(tree->getFullPath(&*iter));
    }
  }

  for (auto iter = tree->nodesBegin(); iter != tree->nodesEnd(); ++iter) {
    appendImageFiles(result, *iter);
  }
}


QStringList InstallerFomod::buildFomodTree(DirectoryTree &tree)
{
  QStringList result;
  const DirectoryTree *fomodTree = findFomodDirectory(&tree);
  for (auto iter = fomodTree->leafsBegin(); iter != fomodTree->leafsEnd(); ++iter) {
    if ((iter->getName() == "info.xml") ||
        (iter->getName() == "ModuleConfig.xml")) {
      result.append(fomodTree->getFullPath(&*iter));
    }
  }

  appendImageFiles(result, &tree);

  return result;
}


IPluginList::PluginStates InstallerFomod::fileState(const QString &fileName)
{
  QString ext = QFileInfo(fileName).suffix().toLower();
  if ((ext == "esp") || (ext == "esm") || (ext == "esl")) {
    IPluginList::PluginStates state = m_MOInfo->pluginList()->state(fileName);
    if (state != IPluginList::STATE_MISSING) {
      return state;
    }
  } else if (allowAnyFile()) {
    QFileInfo info(fileName);
    FileNameString name(info.fileName());
    QStringList files = m_MOInfo->findFiles(
        info.dir().path(), [&, name](const QString &f) -> bool {
          return name == QFileInfo(f).fileName();
        });
    // A note: The list of files produced is somewhat odd as it's the full path
    // to the originating mod (or mods). However, all we care about is if it's
    // there or not.
    if (files.size() != 0) {
      return IPluginList::STATE_ACTIVE;
    }
  } else {
    qWarning() << "A dependency on non esp/esm/esl " << fileName
               << " will always find it as missing";
    return IPluginList::STATE_MISSING;
  }

  // If they are really desparate we look in the full mod list and try that
  if (checkDisabledMods()) {
    IModList *modList = m_MOInfo->modList();
    QStringList list = modList->allMods();
    for (QString mod : list) {
      // Get mod state. if it's active we've already looked. If it's not valid,
      // no point in looking.
      IModList::ModStates state = modList->state(mod);
      if ((state & IModList::STATE_ACTIVE) != 0
          || (state & IModList::STATE_VALID) == 0) {
        continue;
      }
      MOBase::IModInterface *modInfo = m_MOInfo->getMod(mod);
      // Go see if the file is in the mod
      QDir modpath(modInfo->absolutePath());
      QFile file(modpath.absoluteFilePath(fileName));
      if (file.exists()) {
        return IPluginList::STATE_INACTIVE;
      }
    }
  }
  return IPluginList::STATE_MISSING;
}

IPluginInstaller::EInstallResult InstallerFomod::install(GuessedValue<QString> &modName, DirectoryTree &tree,
                                                         QString &version, int &modID)
{
  QStringList installerFiles = buildFomodTree(tree);
  manager()->extractFiles(installerFiles, false);

  try {
    const DirectoryTree *fomodTree = findFomodDirectory(&tree);

    QString fomodPath = fomodTree->getParent()->getFullPath();
    FomodInstallerDialog dialog(modName, fomodPath, std::bind(&InstallerFomod::fileState, this, std::placeholders::_1));
    dialog.initData(m_MOInfo);
    if (!dialog.getVersion().isEmpty()) {
      version = dialog.getVersion();
    }
    if (dialog.getModID() != -1) {
      modID = dialog.getModID();
    }

    manager()->setURL(dialog.getURL());

    if (!dialog.hasOptions()) {
      dialog.transformToSmallInstall();
    }

    if (dialog.exec() == QDialog::Accepted) {
      modName.update(dialog.getName(), GUESS_USER);
      DirectoryTree *newTree = dialog.updateTree(&tree);
      tree = *newTree;
      delete newTree;

      return IPluginInstaller::RESULT_SUCCESS;
    } else {
      if (dialog.manualRequested()) {
        modName.update(dialog.getName(), GUESS_USER);
        return IPluginInstaller::RESULT_MANUALREQUESTED;
      } else {
        return IPluginInstaller::RESULT_FAILED;
      }
    }
  } catch (const std::exception &e) {
    reportError(tr("Installation as fomod failed: %1").arg(e.what()));
    return IPluginInstaller::RESULT_FAILED;
  }
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(installerFomod, InstallerFomod)
#endif


std::vector<unsigned int> InstallerFomod::activeProblems() const
{
  std::vector<unsigned int> result;
  QList<QByteArray> formats = QImageReader::supportedImageFormats();
  if (!formats.contains("jpg")) {
    result.push_back(PROBLEM_IMAGETYPE_UNSUPPORTED);
  }
  return result;
}

QString InstallerFomod::shortDescription(unsigned int key) const
{
  switch (key) {
    case PROBLEM_IMAGETYPE_UNSUPPORTED:
      return tr("image formats not supported.");
    default:
      throw MyException(tr("invalid problem key %1").arg(key));
  }
}

QString InstallerFomod::fullDescription(unsigned int key) const
{
  switch (key) {
    case PROBLEM_IMAGETYPE_UNSUPPORTED:
      return tr("This indicates that files from dlls/imageformats are missing from your MO installation or outdated. "
                "Images in installers may not be displayed. Please re-install MO");
    default:
      throw MyException(tr("invalid problem key %1").arg(key));
  }
}

bool InstallerFomod::hasGuidedFix(unsigned int) const
{
  return false;
}

void InstallerFomod::startGuidedFix(unsigned int) const
{
}
