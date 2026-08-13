#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class NandBuilderController : public QObject {
  Q_OBJECT

  Q_PROPERTY(
      QStringList buildVersions READ buildVersions NOTIFY buildVersionsChanged)
  Q_PROPERTY(QString selectedVersion READ selectedVersion WRITE
                 setSelectedVersion NOTIFY selectedVersionChanged)

  Q_PROPERTY(QStringList imageTypes READ imageTypes NOTIFY imageTypesChanged)
  Q_PROPERTY(QString selectedImageType READ selectedImageType WRITE
                 setSelectedImageType NOTIFY selectedImageTypeChanged)

  Q_PROPERTY(QStringList consoles READ consoles NOTIFY consolesChanged)
  Q_PROPERTY(QString selectedConsole READ selectedConsole WRITE
                 setSelectedConsole NOTIFY selectedConsoleChanged)

  Q_PROPERTY(QStringList xellHacks READ xellHacks NOTIFY xellHacksChanged)
  Q_PROPERTY(QString selectedXellHack READ selectedXellHack WRITE
                 setSelectedXellHack NOTIFY selectedXellHackChanged)

  Q_PROPERTY(QStringList xellImages READ xellImages NOTIFY xellImagesChanged)
  Q_PROPERTY(QString selectedXellImage READ selectedXellImage WRITE
                 setSelectedXellImage NOTIFY selectedXellImageChanged)

  Q_PROPERTY(QStringList simpleVersions READ simpleVersions NOTIFY
                 simpleVersionsChanged)
  Q_PROPERTY(QString selectedSimpleVersion READ selectedSimpleVersion WRITE
                 setSelectedSimpleVersion NOTIFY selectedSimpleVersionChanged)

  Q_PROPERTY(QStringList simpleImageTypes READ simpleImageTypes NOTIFY
                 simpleImageTypesChanged)
  Q_PROPERTY(
      QString selectedSimpleImageType READ selectedSimpleImageType WRITE
          setSelectedSimpleImageType NOTIFY selectedSimpleImageTypeChanged)

  Q_PROPERTY(QStringList simpleHacks READ simpleHacks NOTIFY simpleHacksChanged)
  Q_PROPERTY(QString selectedSimpleHack READ selectedSimpleHack WRITE
                 setSelectedSimpleHack NOTIFY selectedSimpleHackChanged)

  Q_PROPERTY(QStringList smcFiles READ smcFiles NOTIFY smcFilesChanged)
  Q_PROPERTY(QString selectedSmc READ selectedSmc WRITE setSelectedSmc NOTIFY
                 selectedSmcChanged)

  Q_PROPERTY(QStringList availablePatches READ availablePatches NOTIFY
                 availablePatchesChanged)

public:
  explicit NandBuilderController(QObject *parent = nullptr);
  static NandBuilderController &instance();

  QStringList buildVersions() const { return m_buildVersions; }
  QString selectedVersion() const { return m_selectedVersion; }
  Q_INVOKABLE void setSelectedVersion(const QString &version);

  QStringList imageTypes() const { return m_imageTypes; }
  QString selectedImageType() const { return m_selectedImageType; }
  Q_INVOKABLE void setSelectedImageType(const QString &imageType);

  QStringList consoles() const { return m_consoles; }
  QString selectedConsole() const { return m_selectedConsole; }
  Q_INVOKABLE void setSelectedConsole(const QString &console);

  QStringList smcFiles() const { return m_smcFiles; }
  QString selectedSmc() const { return m_selectedSmc; }
  Q_INVOKABLE void setSelectedSmc(const QString &smc);

  QStringList xellHacks() const { return m_xellHacks; }
  QString selectedXellHack() const { return m_selectedXellHack; }
  Q_INVOKABLE void setSelectedXellHack(const QString &hack);

  QStringList xellImages() const { return m_xellImages; }
  QString selectedXellImage() const { return m_selectedXellImage; }
  Q_INVOKABLE void setSelectedXellImage(const QString &image);

  QStringList simpleVersions() const { return m_simpleVersions; }
  QString selectedSimpleVersion() const { return m_selectedSimpleVersion; }
  Q_INVOKABLE void setSelectedSimpleVersion(const QString &version);

  QStringList simpleImageTypes() const { return m_simpleImageTypes; }
  QString selectedSimpleImageType() const { return m_selectedSimpleImageType; }
  Q_INVOKABLE void setSelectedSimpleImageType(const QString &imageType);

  QStringList simpleHacks() const { return m_simpleHacks; }
  QString selectedSimpleHack() const { return m_selectedSimpleHack; }
  Q_INVOKABLE void setSelectedSimpleHack(const QString &hack);

  QStringList availablePatches() const { return m_availablePatches; }

  Q_INVOKABLE QString getResolvedVersion() const;
  Q_INVOKABLE QString getMappedUnderlyingImageType() const;

  Q_INVOKABLE void buildImage(const QVariantMap &config);
  Q_INVOKABLE void refresh();

Q_SIGNALS:
  void buildVersionsChanged();
  void selectedVersionChanged();
  void imageTypesChanged();
  void selectedImageTypeChanged();
  void consolesChanged();
  void selectedConsoleChanged();
  void smcFilesChanged();
  void selectedSmcChanged();
  void xellHacksChanged();
  void selectedXellHackChanged();
  void xellImagesChanged();
  void selectedXellImageChanged();
  void simpleVersionsChanged();
  void selectedSimpleVersionChanged();
  void simpleImageTypesChanged();
  void selectedSimpleImageTypeChanged();
  void simpleHacksChanged();
  void selectedSimpleHackChanged();
  void availablePatchesChanged();

  void buildStarted();
  void buildProgress(int percentage, const QString &status);
  void buildFinished(bool success, const QString &outputPath,
                     const QString &logOutput);

private:
  void scanBuildVersions();
  void scanImageTypes();
  void scanConsoles();
  void scanSmc();
  void scanXellHacks();
  void scanXellImages();
  void scanSimpleVersions();
  void scanSimpleImageTypes();
  void scanSimpleHacks();
  void scanPatches();

  QString m_selectedVersion;
  QString m_selectedImageType;
  QString m_selectedConsole;
  QString m_selectedSmc;

  QString m_selectedXellHack;
  QString m_selectedXellImage;

  QString m_selectedSimpleVersion;
  QString m_selectedSimpleImageType;
  QString m_selectedSimpleHack;

  QStringList m_buildVersions;
  QStringList m_imageTypes;
  QStringList m_consoles;
  QStringList m_smcFiles;

  QStringList m_xellHacks;
  QStringList m_xellImages;

  QStringList m_simpleVersions;
  QStringList m_simpleImageTypes;
  QStringList m_simpleHacks;

  QStringList m_availablePatches;
};
