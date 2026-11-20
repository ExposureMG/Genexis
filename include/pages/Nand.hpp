#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

namespace genexis::pages::nand {

/// QML singleton managing the currently loaded NAND image session.
class NandSession : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(NandSession)
    QML_SINGLETON

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool isNandLoaded READ isNandLoaded NOTIFY isNandLoadedChanged)
    Q_PROPERTY(bool isCpuKeyLoaded READ isCpuKeyLoaded NOTIFY isCpuKeyLoadedChanged)
    Q_PROPERTY(QString loadedFilePath READ loadedFilePath NOTIFY loadedFilePathChanged)
    Q_PROPERTY(QString consoleTarget READ consoleTarget NOTIFY consoleTargetChanged)
    Q_PROPERTY(QString imageSize READ imageSize NOTIFY imageSizeChanged)

public:
    static NandSession &instance();
    static NandSession *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    [[nodiscard]] bool isLoading() const noexcept { return m_isLoading; }
    [[nodiscard]] bool isNandLoaded() const noexcept { return m_isNandLoaded; }
    [[nodiscard]] bool isCpuKeyLoaded() const noexcept { return m_isCpuKeyLoaded; }
    [[nodiscard]] const QString &loadedFilePath() const noexcept { return m_loadedFilePath; }
    [[nodiscard]] const QString &consoleTarget() const noexcept { return m_consoleTarget; }
    [[nodiscard]] const QString &imageSize() const noexcept { return m_imageSize; }

    /// Opens a NAND image file (Phase 1 stub logs path; Phase 3 hooks gxbuild3::driver).
    Q_INVOKABLE void openFile(const QUrl &fileUrl, const QString &cpuKey = {});

    /// Detects a sibling cpukey.txt next to \p fileUrl and returns the uppercased 32-hex key.
    Q_INVOKABLE QString detectCpuKey(const QUrl &fileUrl);

Q_SIGNALS:
    void isLoadingChanged();
    void isNandLoadedChanged();
    void isCpuKeyLoadedChanged();
    void loadedFilePathChanged();
    void consoleTargetChanged();
    void imageSizeChanged();

private:
    explicit NandSession(QObject *parent = nullptr);

    bool m_isLoading{false};
    bool m_isNandLoaded{false};
    bool m_isCpuKeyLoaded{false};
    QString m_loadedFilePath;
    QString m_consoleTarget;
    QString m_imageSize;
};

/// QML singleton exposing parsed NAND overview, bootloader components, and keyvault fields.
class InfoController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(InfoController)
    QML_SINGLETON

    Q_PROPERTY(QString blockType READ blockType NOTIFY infoChanged)
    Q_PROPERTY(QString smcType READ smcType NOTIFY infoChanged)
    Q_PROPERTY(QString kernelVerOrType READ kernelVerOrType NOTIFY infoChanged)

    Q_PROPERTY(QString smcVersion READ smcVersion NOTIFY infoChanged)
    Q_PROPERTY(QString cbVersion READ cbVersion NOTIFY infoChanged)
    Q_PROPERTY(QString cbAVersion READ cbAVersion NOTIFY infoChanged)
    Q_PROPERTY(QString cbBVersion READ cbBVersion NOTIFY infoChanged)
    Q_PROPERTY(QString cbSize READ cbSize NOTIFY infoChanged)
    Q_PROPERTY(QString cbMagic READ cbMagic NOTIFY infoChanged)
    Q_PROPERTY(QString cbLdv READ cbLdv NOTIFY infoChanged)
    Q_PROPERTY(QString cbPairing READ cbPairing NOTIFY infoChanged)
    Q_PROPERTY(QString cbALdv READ cbALdv NOTIFY infoChanged)
    Q_PROPERTY(QString cbAPairing READ cbAPairing NOTIFY infoChanged)
    Q_PROPERTY(QString cdVersion READ cdVersion NOTIFY infoChanged)
    Q_PROPERTY(QString ceVersion READ ceVersion NOTIFY infoChanged)
    Q_PROPERTY(QString cf0Version READ cf0Version NOTIFY infoChanged)
    Q_PROPERTY(QString cg0Version READ cg0Version NOTIFY infoChanged)
    Q_PROPERTY(QString cf0Ldv READ cf0Ldv NOTIFY infoChanged)
    Q_PROPERTY(QString cf0Pairing READ cf0Pairing NOTIFY infoChanged)
    Q_PROPERTY(QString cf1Version READ cf1Version NOTIFY infoChanged)
    Q_PROPERTY(QString cg1Version READ cg1Version NOTIFY infoChanged)
    Q_PROPERTY(QString cf1Ldv READ cf1Ldv NOTIFY infoChanged)
    Q_PROPERTY(QString cf1Pairing READ cf1Pairing NOTIFY infoChanged)

    Q_PROPERTY(QString serialNumber READ serialNumber NOTIFY infoChanged)
    Q_PROPERTY(QString consoleId READ consoleId NOTIFY infoChanged)
    Q_PROPERTY(QString dvdKey READ dvdKey NOTIFY infoChanged)
    Q_PROPERTY(QString gameRegion READ gameRegion NOTIFY infoChanged)
    Q_PROPERTY(QString consoleType READ consoleType NOTIFY infoChanged)

    Q_PROPERTY(QVariantList components READ components NOTIFY infoChanged)

public:
    static InfoController &instance();
    static InfoController *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    [[nodiscard]] const QString &blockType() const noexcept { return m_blockType; }
    [[nodiscard]] const QString &smcType() const noexcept { return m_smcType; }
    [[nodiscard]] const QString &kernelVerOrType() const noexcept { return m_kernelVerOrType; }

    [[nodiscard]] const QString &smcVersion() const noexcept { return m_smcVersion; }
    [[nodiscard]] const QString &cbVersion() const noexcept { return m_cbVersion; }
    [[nodiscard]] const QString &cbAVersion() const noexcept { return m_cbAVersion; }
    [[nodiscard]] const QString &cbBVersion() const noexcept { return m_cbBVersion; }
    [[nodiscard]] const QString &cbSize() const noexcept { return m_cbSize; }
    [[nodiscard]] const QString &cbMagic() const noexcept { return m_cbMagic; }
    [[nodiscard]] const QString &cbLdv() const noexcept { return m_cbLdv; }
    [[nodiscard]] const QString &cbPairing() const noexcept { return m_cbPairing; }
    [[nodiscard]] const QString &cbALdv() const noexcept { return m_cbALdv; }
    [[nodiscard]] const QString &cbAPairing() const noexcept { return m_cbAPairing; }
    [[nodiscard]] const QString &cdVersion() const noexcept { return m_cdVersion; }
    [[nodiscard]] const QString &ceVersion() const noexcept { return m_ceVersion; }
    [[nodiscard]] const QString &cf0Version() const noexcept { return m_cf0Version; }
    [[nodiscard]] const QString &cg0Version() const noexcept { return m_cg0Version; }
    [[nodiscard]] const QString &cf0Ldv() const noexcept { return m_cf0Ldv; }
    [[nodiscard]] const QString &cf0Pairing() const noexcept { return m_cf0Pairing; }
    [[nodiscard]] const QString &cf1Version() const noexcept { return m_cf1Version; }
    [[nodiscard]] const QString &cg1Version() const noexcept { return m_cg1Version; }
    [[nodiscard]] const QString &cf1Ldv() const noexcept { return m_cf1Ldv; }
    [[nodiscard]] const QString &cf1Pairing() const noexcept { return m_cf1Pairing; }

    [[nodiscard]] const QString &serialNumber() const noexcept { return m_serialNumber; }
    [[nodiscard]] const QString &consoleId() const noexcept { return m_consoleId; }
    [[nodiscard]] const QString &dvdKey() const noexcept { return m_dvdKey; }
    [[nodiscard]] const QString &gameRegion() const noexcept { return m_gameRegion; }
    [[nodiscard]] const QString &consoleType() const noexcept { return m_consoleType; }

    [[nodiscard]] const QVariantList &components() const noexcept { return m_components; }

Q_SIGNALS:
    void infoChanged();

private:
    explicit InfoController(QObject *parent = nullptr);

    QString m_blockType;
    QString m_smcType;
    QString m_kernelVerOrType;

    QString m_smcVersion;
    QString m_cbVersion;
    QString m_cbAVersion;
    QString m_cbBVersion;
    QString m_cbSize;
    QString m_cbMagic;
    QString m_cbLdv;
    QString m_cbPairing;
    QString m_cbALdv;
    QString m_cbAPairing;
    QString m_cdVersion;
    QString m_ceVersion;
    QString m_cf0Version;
    QString m_cg0Version;
    QString m_cf0Ldv;
    QString m_cf0Pairing;
    QString m_cf1Version;
    QString m_cg1Version;
    QString m_cf1Ldv;
    QString m_cf1Pairing;

    QString m_serialNumber;
    QString m_consoleId;
    QString m_dvdKey;
    QString m_gameRegion;
    QString m_consoleType;

    QVariantList m_components;
};

/// QML singleton exposing NAND builder options and build execution (stubbed in Phase 1).
class BuilderController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(BuilderController)
    QML_SINGLETON

    Q_PROPERTY(QStringList availablePatches READ availablePatches NOTIFY availablePatchesChanged)
    Q_PROPERTY(QStringList simpleVersions READ simpleVersions NOTIFY simpleVersionsChanged)
    Q_PROPERTY(QStringList simpleImageTypes READ simpleImageTypes NOTIFY simpleImageTypesChanged)
    Q_PROPERTY(QStringList simpleHacks READ simpleHacks NOTIFY simpleHacksChanged)
    Q_PROPERTY(QStringList xellHacks READ xellHacks NOTIFY xellHacksChanged)
    Q_PROPERTY(QStringList xellImages READ xellImages NOTIFY xellImagesChanged)
    Q_PROPERTY(QStringList buildVersions READ buildVersions NOTIFY buildVersionsChanged)
    Q_PROPERTY(QStringList imageTypes READ imageTypes NOTIFY imageTypesChanged)
    Q_PROPERTY(QStringList consoles READ consoles NOTIFY consolesChanged)
    Q_PROPERTY(QStringList smcFiles READ smcFiles NOTIFY smcFilesChanged)
    Q_PROPERTY(int defaultCbLdv READ defaultCbLdv NOTIFY defaultCbLdvChanged)
    Q_PROPERTY(QString defaultPairingData READ defaultPairingData NOTIFY defaultPairingDataChanged)

public:
    static BuilderController &instance();
    static BuilderController *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    [[nodiscard]] const QStringList &availablePatches() const noexcept { return m_availablePatches; }
    [[nodiscard]] const QStringList &simpleVersions() const noexcept { return m_simpleVersions; }
    [[nodiscard]] const QStringList &simpleImageTypes() const noexcept { return m_simpleImageTypes; }
    [[nodiscard]] const QStringList &simpleHacks() const noexcept { return m_simpleHacks; }
    [[nodiscard]] const QStringList &xellHacks() const noexcept { return m_xellHacks; }
    [[nodiscard]] const QStringList &xellImages() const noexcept { return m_xellImages; }
    [[nodiscard]] const QStringList &buildVersions() const noexcept { return m_buildVersions; }
    [[nodiscard]] const QStringList &imageTypes() const noexcept { return m_imageTypes; }
    [[nodiscard]] const QStringList &consoles() const noexcept { return m_consoles; }
    [[nodiscard]] const QStringList &smcFiles() const noexcept { return m_smcFiles; }
    [[nodiscard]] int defaultCbLdv() const noexcept { return m_defaultCbLdv; }
    [[nodiscard]] const QString &defaultPairingData() const noexcept { return m_defaultPairingData; }

    Q_INVOKABLE void setSelectedSimpleVersion(const QString &version);
    Q_INVOKABLE void setSelectedSimpleImageType(const QString &imageType);
    Q_INVOKABLE void setSelectedSimpleHack(const QString &hack);
    Q_INVOKABLE void setSelectedXellHack(const QString &hack);
    Q_INVOKABLE void setSelectedXellImage(const QString &image);
    Q_INVOKABLE void setSelectedVersion(const QString &version);
    Q_INVOKABLE void setSelectedImageType(const QString &imageType);
    Q_INVOKABLE void setSelectedConsole(const QString &console);
    Q_INVOKABLE void setSelectedSmc(const QString &smc);
    Q_INVOKABLE void buildImage(const QVariantMap &config);

Q_SIGNALS:
    void availablePatchesChanged();
    void simpleVersionsChanged();
    void simpleImageTypesChanged();
    void simpleHacksChanged();
    void xellHacksChanged();
    void xellImagesChanged();
    void buildVersionsChanged();
    void imageTypesChanged();
    void consolesChanged();
    void smcFilesChanged();
    void defaultCbLdvChanged();
    void defaultPairingDataChanged();

    void buildStarted();
    void buildProgress(int percentage, const QString &status);
    void buildFinished(bool success, const QString &outputPath, const QString &logOutput);

private:
    explicit BuilderController(QObject *parent = nullptr);

    QStringList m_availablePatches;
    QStringList m_simpleVersions;
    QStringList m_simpleImageTypes;
    QStringList m_simpleHacks;
    QStringList m_xellHacks;
    QStringList m_xellImages;
    QStringList m_buildVersions;
    QStringList m_imageTypes;
    QStringList m_consoles;
    QStringList m_smcFiles;
    int m_defaultCbLdv{1};
    QString m_defaultPairingData{QStringLiteral("0x000000")};
};

} // namespace genexis::pages::nand
