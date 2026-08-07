#include "pages/NandBuilderController.hpp"
#include "StartupManager.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>

NandBuilderController::NandBuilderController(QObject *parent)
    : QObject(parent)
{
    refresh();
}

NandBuilderController &NandBuilderController::instance()
{
    static NandBuilderController inst;
    return inst;
}

QString NandBuilderController::getXeBuildDataPath() const
{
    QString appData = StartupManager::instance().appDataPath();
    return QDir(appData).filePath(QStringLiteral("data/nand/xebuild"));
}

QString NandBuilderController::getRgBuildDataPath() const
{
    QString appData = StartupManager::instance().appDataPath();
    return QDir(appData).filePath(QStringLiteral("data/nand/rgbuild"));
}

QString NandBuilderController::getXellDataPath() const
{
    QString appData = StartupManager::instance().appDataPath();
    return QDir(appData).filePath(QStringLiteral("data/xell-images"));
}

QString NandBuilderController::findRgBuildVersionFolder(const QString &version) const
{
    QString rgPath = getRgBuildDataPath();
    QDir defaultsDir(QDir(rgPath).filePath(QStringLiteral("builds/defaults")));
    QDir filesystemsDir(QDir(rgPath).filePath(QStringLiteral("filesystems")));

    if (!defaultsDir.exists() || !filesystemsDir.exists()) {
        return QString();
    }

    QString candidateDev = version + QStringLiteral("-dev");
    if (QFileInfo::exists(defaultsDir.filePath(candidateDev)) && QFileInfo::exists(filesystemsDir.filePath(candidateDev))) {
        return candidateDev;
    }

    if (QFileInfo::exists(defaultsDir.filePath(version)) && QFileInfo::exists(filesystemsDir.filePath(version))) {
        return version;
    }

    return QString();
}

void NandBuilderController::refresh()
{
    scanBuildVersions();
    scanXellHacks();
    scanPatches();
}

void NandBuilderController::setSelectedVersion(const QString &version)
{
    if (m_selectedVersion == version) {
        return;
    }
    m_selectedVersion = version;
    Q_EMIT selectedVersionChanged();
    scanImageTypes();
    scanPatches();
}

void NandBuilderController::setSelectedImageType(const QString &imageType)
{
    if (m_selectedImageType == imageType) {
        return;
    }
    m_selectedImageType = imageType;
    Q_EMIT selectedImageTypeChanged();
    scanConsoles();
}

void NandBuilderController::setSelectedConsole(const QString &console)
{
    if (m_selectedConsole == console) {
        return;
    }
    m_selectedConsole = console;
    Q_EMIT selectedConsoleChanged();
}

void NandBuilderController::scanBuildVersions()
{
    m_buildVersions.clear();

    // 1. Discover xeBuild versions
    QString xePath = getXeBuildDataPath();
    QDir xeDir(xePath);
    if (xeDir.exists()) {
        QFileInfoList entries = xeDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry : entries) {
            if (!m_buildVersions.contains(entry.fileName())) {
                m_buildVersions.append(entry.fileName());
            }
        }
    }

    // 2. Discover RGBuild versions (must exist in both builds/defaults/ AND filesystems/)
    QString rgPath = getRgBuildDataPath();
    QDir defaultsDir(QDir(rgPath).filePath(QStringLiteral("builds/defaults")));
    QDir filesystemsDir(QDir(rgPath).filePath(QStringLiteral("filesystems")));

    if (defaultsDir.exists() && filesystemsDir.exists()) {
        QFileInfoList entries = defaultsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry : entries) {
            QString folderName = entry.fileName();
            if (QFileInfo::exists(filesystemsDir.filePath(folderName))) {
                QString verName = folderName;
                if (verName.endsWith(QStringLiteral("-dev"), Qt::CaseInsensitive)) {
                    verName.chop(4);
                }
                if (!m_buildVersions.contains(verName)) {
                    m_buildVersions.append(verName);
                }
            }
        }
    }

    // Sort build versions descending numerically (e.g. 17559 first)
    std::sort(m_buildVersions.begin(), m_buildVersions.end(), [](const QString &a, const QString &b) {
        bool okA = false, okB = false;
        int numA = a.toInt(&okA);
        int numB = b.toInt(&okB);
        if (okA && okB) {
            return numA > numB;
        }
        return a > b;
    });

    Q_EMIT buildVersionsChanged();

    if (!m_buildVersions.contains(m_selectedVersion)) {
        m_selectedVersion = m_buildVersions.isEmpty() ? QString() : m_buildVersions.first();
        Q_EMIT selectedVersionChanged();
    }

    scanImageTypes();
    scanSimpleVersions();
}

void NandBuilderController::scanImageTypes()
{
    m_imageTypes.clear();

    if (!m_selectedVersion.isEmpty()) {
        // 1. Discover xeBuild image types
        QString verPath = QDir(getXeBuildDataPath()).filePath(m_selectedVersion);
        QDir dir(verPath);
        if (dir.exists()) {
            QFileInfoList entries = dir.entryInfoList(QStringList{QStringLiteral("_*.ini")}, QDir::Files);
            for (const auto &entry : entries) {
                QString name = entry.fileName();
                if (name.startsWith(u'_') && name.endsWith(QStringLiteral(".ini"), Qt::CaseInsensitive)) {
                    QString typeName = name.mid(1, name.length() - 5);
                    if (!m_imageTypes.contains(typeName)) {
                        m_imageTypes.append(typeName);
                    }
                }
            }
        }

        // 2. Discover RGBuild image types
        QString rgFolder = findRgBuildVersionFolder(m_selectedVersion);
        if (!rgFolder.isEmpty()) {
            QString rgVerPath = QDir(getRgBuildDataPath()).filePath(QStringLiteral("builds/defaults/") + rgFolder);
            QDir rgDir(rgVerPath);

            if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fat.ini")))) {
                if (!m_imageTypes.contains(QStringLiteral("RGL-Glitch1"))) {
                    m_imageTypes.append(QStringLiteral("RGL-Glitch1"));
                }
            }
            if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fatRGH2.ini"))) || QFileInfo::exists(rgDir.filePath(QStringLiteral("slim.ini")))) {
                if (!m_imageTypes.contains(QStringLiteral("RGL-Glitch2"))) {
                    m_imageTypes.append(QStringLiteral("RGL-Glitch2"));
                }
            }
        }

        m_imageTypes.sort(Qt::CaseInsensitive);
    }

    Q_EMIT imageTypesChanged();

    if (!m_imageTypes.contains(m_selectedImageType)) {
        m_selectedImageType = m_imageTypes.isEmpty() ? QString() : m_imageTypes.first();
        Q_EMIT selectedImageTypeChanged();
    }

    scanConsoles();
}

void NandBuilderController::scanConsoles()
{
    m_consoles.clear();

    if (!m_selectedVersion.isEmpty() && !m_selectedImageType.isEmpty()) {
        if (m_selectedImageType == QStringLiteral("RGL-Glitch1") || m_selectedImageType == QStringLiteral("RGL-Glitch2")) {
            // RGBuild Glitch Console Target Resolution
            QString rgFolder = findRgBuildVersionFolder(m_selectedVersion);
            if (!rgFolder.isEmpty()) {
                QString rgVerPath = QDir(getRgBuildDataPath()).filePath(QStringLiteral("builds/defaults/") + rgFolder);
                QDir rgDir(rgVerPath);

                if (m_selectedImageType == QStringLiteral("RGL-Glitch1")) {
                    if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fat.ini")))) {
                        m_consoles.append(QStringList{QStringLiteral("Zephyr"), QStringLiteral("Falcon"), QStringLiteral("Jasper")});
                    }
                } else if (m_selectedImageType == QStringLiteral("RGL-Glitch2")) {
                    if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fatRGH2.ini")))) {
                        m_consoles.append(QStringList{QStringLiteral("Zephyr"), QStringLiteral("Falcon"), QStringLiteral("Jasper")});
                    }
                    if (QFileInfo::exists(rgDir.filePath(QStringLiteral("slim.ini")))) {
                        m_consoles.append(QStringList{QStringLiteral("Trinity"), QStringLiteral("Corona")});
                    }
                }
            }
        } else {
            // xeBuild Console Target Resolution
            QString iniName = QStringLiteral("_") + m_selectedImageType + QStringLiteral(".ini");
            QString iniPath = QDir(getXeBuildDataPath()).filePath(m_selectedVersion + QStringLiteral("/") + iniName);

            QFile iniFile(iniPath);
            if (iniFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream stream(&iniFile);
                while (!stream.atEnd()) {
                    QString line = stream.readLine().trimmed();
                    if (line.startsWith(u'[') && line.endsWith(u']')) {
                        QString section = line.mid(1, line.length() - 2).trimmed();
                        QString secLower = section.toLower();
                        if (secLower != QStringLiteral("version") &&
                            secLower != QStringLiteral("security") &&
                            secLower != QStringLiteral("rawpatch") &&
                            secLower != QStringLiteral("flashfs")) {
                            if (!m_consoles.contains(section)) {
                                m_consoles.append(section);
                            }
                        }
                    }
                }
                iniFile.close();
            }
        }
    }

    Q_EMIT consolesChanged();

    if (!m_consoles.contains(m_selectedConsole)) {
        m_selectedConsole = m_consoles.isEmpty() ? QString() : m_consoles.first();
        Q_EMIT selectedConsoleChanged();
    }
}

void NandBuilderController::setSelectedXellHack(const QString &hack)
{
    if (m_selectedXellHack == hack) {
        return;
    }
    m_selectedXellHack = hack;
    Q_EMIT selectedXellHackChanged();
    scanXellImages();
}

void NandBuilderController::setSelectedXellImage(const QString &image)
{
    if (m_selectedXellImage == image) {
        return;
    }
    m_selectedXellImage = image;
    Q_EMIT selectedXellImageChanged();
}

void NandBuilderController::scanXellHacks()
{
    m_xellHacks.clear();
    QString xellPath = getXellDataPath();
    QDir dir(xellPath);

    if (dir.exists()) {
        QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry : entries) {
            m_xellHacks.append(entry.fileName());
        }
        m_xellHacks.sort(Qt::CaseInsensitive);
    }

    Q_EMIT xellHacksChanged();

    if (!m_xellHacks.contains(m_selectedXellHack)) {
        m_selectedXellHack = m_xellHacks.isEmpty() ? QString() : m_xellHacks.first();
        Q_EMIT selectedXellHackChanged();
    }

    scanXellImages();
}

void NandBuilderController::scanXellImages()
{
    m_xellImages.clear();

    if (!m_selectedXellHack.isEmpty()) {
        QString hackPath = QDir(getXellDataPath()).filePath(m_selectedXellHack);
        QDir dir(hackPath);
        if (dir.exists()) {
            QFileInfoList entries = dir.entryInfoList(QDir::Files);
            for (const auto &entry : entries) {
                QString name = entry.completeBaseName();
                if (!m_xellImages.contains(name)) {
                    m_xellImages.append(name);
                }
            }
            m_xellImages.sort(Qt::CaseInsensitive);
        }
    }

    Q_EMIT xellImagesChanged();

    if (!m_xellImages.contains(m_selectedXellImage)) {
        m_selectedXellImage = m_xellImages.isEmpty() ? QString() : m_xellImages.first();
        Q_EMIT selectedXellImageChanged();
    }
}

void NandBuilderController::setSelectedSimpleVersion(const QString &version)
{
    if (m_selectedSimpleVersion == version) {
        return;
    }
    m_selectedSimpleVersion = version;
    Q_EMIT selectedSimpleVersionChanged();
    scanSimpleImageTypes();
    scanPatches();
}

void NandBuilderController::setSelectedSimpleImageType(const QString &imageType)
{
    if (m_selectedSimpleImageType == imageType) {
        return;
    }
    m_selectedSimpleImageType = imageType;
    Q_EMIT selectedSimpleImageTypeChanged();
    scanSimpleHacks();
}

void NandBuilderController::setSelectedSimpleHack(const QString &hack)
{
    if (m_selectedSimpleHack == hack) {
        return;
    }
    m_selectedSimpleHack = hack;
    Q_EMIT selectedSimpleHackChanged();
}

QString NandBuilderController::getResolvedVersion() const
{
    if (m_selectedSimpleVersion.isEmpty() || m_selectedSimpleVersion == QStringLiteral("Latest")) {
        return m_buildVersions.isEmpty() ? QString() : m_buildVersions.first();
    }
    return m_selectedSimpleVersion;
}

QString NandBuilderController::getMappedUnderlyingImageType() const
{
    if (m_selectedSimpleImageType == QStringLiteral("Retail")) {
        return QStringLiteral("retail");
    }
    if (m_selectedSimpleImageType == QStringLiteral("FreeBoot")) {
        if (m_selectedSimpleHack == QStringLiteral("RGH 1")) return QStringLiteral("glitch");
        if (m_selectedSimpleHack == QStringLiteral("RGH 3") || m_selectedSimpleHack == QStringLiteral("RGH 1.3")) return QStringLiteral("glitch3");
        return QStringLiteral("glitch2");
    }
    if (m_selectedSimpleImageType == QStringLiteral("RGLoader")) {
        if (m_selectedSimpleHack == QStringLiteral("RGH 1")) return QStringLiteral("RGL-Glitch1");
        return QStringLiteral("RGL-Glitch2");
    }
    return QStringLiteral("retail");
}

void NandBuilderController::scanSimpleVersions()
{
    m_simpleVersions.clear();
    m_simpleVersions.append(QStringLiteral("Latest"));
    for (const auto &ver : m_buildVersions) {
        m_simpleVersions.append(ver);
    }

    Q_EMIT simpleVersionsChanged();

    if (!m_simpleVersions.contains(m_selectedSimpleVersion)) {
        m_selectedSimpleVersion = QStringLiteral("Latest");
        Q_EMIT selectedSimpleVersionChanged();
    }

    scanSimpleImageTypes();
}

void NandBuilderController::scanSimpleImageTypes()
{
    m_simpleImageTypes.clear();

    if (m_selectedSimpleVersion == QStringLiteral("Latest") || m_selectedSimpleVersion.isEmpty()) {
        m_simpleImageTypes = QStringList{QStringLiteral("Retail"), QStringLiteral("FreeBoot"), QStringLiteral("RGLoader")};
    } else {
        QString resVer = getResolvedVersion();
        if (!resVer.isEmpty()) {
            // Check xeBuild
            QString verPath = QDir(getXeBuildDataPath()).filePath(resVer);
            QDir dir(verPath);
            if (dir.exists()) {
                if (QFileInfo::exists(dir.filePath(QStringLiteral("_retail.ini")))) {
                    m_simpleImageTypes.append(QStringLiteral("Retail"));
                }
                QFileInfoList entries = dir.entryInfoList(QStringList{QStringLiteral("_glitch*.ini")}, QDir::Files);
                if (!entries.isEmpty()) {
                    m_simpleImageTypes.append(QStringLiteral("FreeBoot"));
                }
            }

            // Check RGBuild
            QString rgFolder = findRgBuildVersionFolder(resVer);
            if (!rgFolder.isEmpty()) {
                QString rgVerPath = QDir(getRgBuildDataPath()).filePath(QStringLiteral("builds/defaults/") + rgFolder);
                QDir rgDir(rgVerPath);
                if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fat.ini"))) ||
                    QFileInfo::exists(rgDir.filePath(QStringLiteral("fatRGH2.ini"))) ||
                    QFileInfo::exists(rgDir.filePath(QStringLiteral("slim.ini")))) {
                    if (!m_simpleImageTypes.contains(QStringLiteral("RGLoader"))) {
                        m_simpleImageTypes.append(QStringLiteral("RGLoader"));
                    }
                }
            }
        }
    }

    Q_EMIT simpleImageTypesChanged();

    if (!m_simpleImageTypes.contains(m_selectedSimpleImageType)) {
        m_selectedSimpleImageType = m_simpleImageTypes.contains(QStringLiteral("FreeBoot"))
                                       ? QStringLiteral("FreeBoot")
                                       : (m_simpleImageTypes.isEmpty() ? QString() : m_simpleImageTypes.first());
        Q_EMIT selectedSimpleImageTypeChanged();
    }

    scanSimpleHacks();
}

void NandBuilderController::scanSimpleHacks()
{
    m_simpleHacks.clear();

    if (m_selectedSimpleImageType == QStringLiteral("Retail")) {
        m_simpleHacks.append(QStringLiteral("None"));
    } else if (m_selectedSimpleImageType == QStringLiteral("FreeBoot")) {
        if (m_selectedSimpleVersion == QStringLiteral("Latest") || m_selectedSimpleVersion.isEmpty()) {
            m_simpleHacks = QStringList{
                QStringLiteral("RGH 1"),
                QStringLiteral("RGH 1.2"),
                QStringLiteral("RGH 2"),
                QStringLiteral("S-RGH"),
                QStringLiteral("RGH 3"),
                QStringLiteral("RGH 1.3")
            };
        } else {
            QString resVer = getResolvedVersion();
            QString verPath = QDir(getXeBuildDataPath()).filePath(resVer);
            QDir dir(verPath);
            if (dir.exists()) {
                if (QFileInfo::exists(dir.filePath(QStringLiteral("_glitch.ini")))) {
                    m_simpleHacks.append(QStringLiteral("RGH 1"));
                }
                if (QFileInfo::exists(dir.filePath(QStringLiteral("_glitch2.ini"))) ||
                    QFileInfo::exists(dir.filePath(QStringLiteral("_glitch2m.ini")))) {
                    m_simpleHacks.append(QStringList{QStringLiteral("RGH 1.2"), QStringLiteral("RGH 2"), QStringLiteral("S-RGH")});
                }
                if (QFileInfo::exists(dir.filePath(QStringLiteral("_glitch3.ini")))) {
                    m_simpleHacks.append(QStringList{QStringLiteral("RGH 3"), QStringLiteral("RGH 1.3")});
                }
            }
        }
    } else if (m_selectedSimpleImageType == QStringLiteral("RGLoader")) {
        if (m_selectedSimpleVersion == QStringLiteral("Latest") || m_selectedSimpleVersion.isEmpty()) {
            m_simpleHacks = QStringList{
                QStringLiteral("RGH 1"),
                QStringLiteral("RGH 1.2"),
                QStringLiteral("RGH 2"),
                QStringLiteral("S-RGH")
            };
        } else {
            QString rgFolder = findRgBuildVersionFolder(getResolvedVersion());
            if (!rgFolder.isEmpty()) {
                QString rgVerPath = QDir(getRgBuildDataPath()).filePath(QStringLiteral("builds/defaults/") + rgFolder);
                QDir rgDir(rgVerPath);
                if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fat.ini")))) {
                    m_simpleHacks.append(QStringLiteral("RGH 1"));
                }
                if (QFileInfo::exists(rgDir.filePath(QStringLiteral("fatRGH2.ini"))) ||
                    QFileInfo::exists(rgDir.filePath(QStringLiteral("slim.ini")))) {
                    m_simpleHacks.append(QStringList{QStringLiteral("RGH 1.2"), QStringLiteral("RGH 2"), QStringLiteral("S-RGH")});
                }
            }
        }
    }

    Q_EMIT simpleHacksChanged();

    if (!m_simpleHacks.contains(m_selectedSimpleHack)) {
        m_selectedSimpleHack = m_simpleHacks.isEmpty() ? QString() : m_simpleHacks.first();
        Q_EMIT selectedSimpleHackChanged();
    }
}

void NandBuilderController::scanPatches()
{
    m_availablePatches.clear();

    QString verToScan = getResolvedVersion();
    if (verToScan.isEmpty()) {
        verToScan = m_selectedVersion;
    }

    if (!verToScan.isEmpty()) {
        QString binPath = QDir(getXeBuildDataPath()).filePath(verToScan + QStringLiteral("/bin"));
        QDir binDir(binPath);
        if (binDir.exists()) {
            QFileInfoList entries = binDir.entryInfoList(QStringList{QStringLiteral("*.bin")}, QDir::Files);
            for (const auto &entry : entries) {
                QString fname = entry.fileName();
                if (!fname.startsWith(QStringLiteral("patches_"), Qt::CaseInsensitive)) {
                    QString patchName = entry.completeBaseName();
                    if (!m_availablePatches.contains(patchName)) {
                        m_availablePatches.append(patchName);
                    }
                }
            }
            m_availablePatches.sort(Qt::CaseInsensitive);
        }
    }

    Q_EMIT availablePatchesChanged();
}
