#pragma once

#include "backend/IBuilderService.hpp"

#include <QDir>
#include <QFileInfo>
#include <QMetaType>
#include <QString>
#include <QVariantMap>

#include <filesystem>
#include <initializer_list>
#include <string>

namespace gxapi::pages::detail {

struct NandBuildConfigDefaults {
  QString cpuKeyHex;
  QString version;
  QString imageType;
  QString consoleModel;
  QString outputPath;
  QString sourceNandPath;
  QString smcSelection;
  QString smcDataRoot;
};

inline bool isDonorBuildRequest(const QVariantMap &values) {
  return values.value(QStringLiteral("donorMode")).toBool() ||
         values.value(QStringLiteral("mode"))
                 .toString()
                 .trimmed()
                 .compare(QStringLiteral("donor"), Qt::CaseInsensitive) == 0;
}

inline QString firstNonEmpty(const QVariantMap &values,
                             std::initializer_list<QString> keys) {
  for (const auto &key : keys) {
    const QString value = values.value(key).toString().trimmed();
    if (!value.isEmpty()) {
      return value;
    }
  }
  return {};
}

inline void setOptionalPath(std::optional<std::filesystem::path> &destination,
                            const QString &value) {
  if (!value.isEmpty()) {
    destination = std::filesystem::path(value.toStdString());
  }
}

inline backend::NandBuildConfig
normalizeNandBuildConfig(const QVariantMap &values,
                         const NandBuildConfigDefaults &defaults) {
  backend::NandBuildConfig result;
  const QVariantMap options = values.value(QStringLiteral("options")).toMap();

  const QString buildType =
      values.value(QStringLiteral("buildType")).toString().trimmed();
  result.xellOnly = values.value(QStringLiteral("xellOnly")).toBool() ||
                    buildType == QStringLiteral("XeLL Image");

  QString version = firstNonEmpty(
      values, {QStringLiteral("buildVersion"), QStringLiteral("version")});
  if (version.isEmpty() || version == QStringLiteral("Latest")) {
    version = defaults.version;
  }
  result.version = version.toStdString();

  const bool simpleRequest = values.contains(QStringLiteral("buildVersion")) ||
                             values.contains(QStringLiteral("hackVersion"));
  QString imageType =
      values.value(QStringLiteral("imageType")).toString().trimmed();
  if ((simpleRequest && !defaults.imageType.isEmpty()) || imageType.isEmpty()) {
    imageType = defaults.imageType;
  }
  result.imageType = imageType.toStdString();

  QString consoleModel = firstNonEmpty(
      values, {QStringLiteral("consoleModel"), QStringLiteral("console")});
  if (consoleModel.isEmpty()) {
    consoleModel = defaults.consoleModel;
  }
  result.consoleModel = consoleModel.toStdString();

  QString cpuKeyHex =
      values.value(QStringLiteral("cpuKey")).toString().trimmed();
  if (cpuKeyHex.isEmpty()) {
    cpuKeyHex = options.value(QStringLiteral("cpuKey")).toString().trimmed();
  }
  if (cpuKeyHex.isEmpty()) {
    cpuKeyHex = defaults.cpuKeyHex.trimmed();
  }
  result.cpuKeyHex = cpuKeyHex.toStdString();

  QString outputPath =
      values.value(QStringLiteral("outputPath")).toString().trimmed();
  if (outputPath.isEmpty()) {
    outputPath = defaults.outputPath;
  }
  result.outputPath = outputPath.toStdString();

  setOptionalPath(
      result.sourceNandPath,
      values.value(QStringLiteral("sourceNandPath")).toString().trimmed());
  if (!result.sourceNandPath && !isDonorBuildRequest(values)) {
    setOptionalPath(result.sourceNandPath, defaults.sourceNandPath.trimmed());
  }

  QString customKvPath = firstNonEmpty(
      values, {QStringLiteral("customKvPath"), QStringLiteral("keyvaultPath")});
  if (customKvPath.isEmpty()) {
    customKvPath = firstNonEmpty(
        options, {QStringLiteral("kvPath"), QStringLiteral("keyvaultPath")});
  }
  setOptionalPath(result.customKvPath, customKvPath);

  QString customSmcPath =
      values.value(QStringLiteral("customSmcPath")).toString().trimmed();
  if (customSmcPath.isEmpty()) {
    customSmcPath =
        options.value(QStringLiteral("smcPath")).toString().trimmed();
  }
  if (customSmcPath.isEmpty()) {
    QString smcSelection = firstNonEmpty(
        values, {QStringLiteral("smc"), QStringLiteral("smcFile")});
    if (smcSelection.isEmpty()) {
      smcSelection = defaults.smcSelection;
    }
    if (!smcSelection.isEmpty()) {
      if (QFileInfo(smcSelection).isAbsolute()) {
        customSmcPath = smcSelection;
      } else if (!defaults.smcDataRoot.isEmpty() && !consoleModel.isEmpty()) {
        customSmcPath = QDir(defaults.smcDataRoot)
                            .filePath(consoleModel + u'/' + smcSelection);
      }
    }
  }
  if (!customSmcPath.isEmpty() && QFileInfo::exists(customSmcPath)) {
    setOptionalPath(result.customSmcPath, customSmcPath);
  } else if (!options.value(QStringLiteral("smcPath"))
                  .toString()
                  .trimmed()
                  .isEmpty() ||
             !values.value(QStringLiteral("customSmcPath"))
                  .toString()
                  .trimmed()
                  .isEmpty()) {
    setOptionalPath(result.customSmcPath, customSmcPath);
  }

  setOptionalPath(
      result.customSmcConfigPath,
      values.value(QStringLiteral("customSmcConfigPath")).toString().trimmed());
  setOptionalPath(
      result.xboxupdPath,
      values.value(QStringLiteral("xboxupdPath")).toString().trimmed());

  const QVariantList patches = values.value(QStringLiteral("patches")).toList();
  result.patches.reserve(static_cast<std::size_t>(patches.size()));
  for (const auto &patch : patches) {
    const QString selected = patch.toString().trimmed();
    if (!selected.isEmpty()) {
      result.patches.push_back(selected.toStdString());
    }
  }

  if (values.contains(QStringLiteral("cfLdv"))) {
    result.rawOptions.emplace_back(
        "cfldv",
        values.value(QStringLiteral("cfLdv")).toString().toStdString());
  }
  if (values.contains(QStringLiteral("cbLdv"))) {
    result.rawOptions.emplace_back(
        "cbldv",
        values.value(QStringLiteral("cbLdv")).toString().toStdString());
  }
  const QString pairingData = firstNonEmpty(
      values, {QStringLiteral("pairingData"), QStringLiteral("pairing_data")});
  if (!pairingData.isEmpty()) {
    result.rawOptions.emplace_back("pairing_data", pairingData.toStdString());
  }
  for (auto it = options.cbegin(); it != options.cend(); ++it) {
    if (it.key() == QStringLiteral("cpuKey") ||
        it.key() == QStringLiteral("kvPath") ||
        it.key() == QStringLiteral("keyvaultPath") ||
        it.key() == QStringLiteral("smcPath") ||
        it.key() == QStringLiteral("customArgs")) {
      continue;
    }
    if (it.value().typeId() == QMetaType::Bool) {
      if (it.value().toBool()) {
        result.rawOptions.emplace_back(it.key().toStdString(), "");
      }
      continue;
    }
    const QString optionValue = it.value().toString().trimmed();
    if (!optionValue.isEmpty()) {
      result.rawOptions.emplace_back(it.key().toStdString(),
                                     optionValue.toStdString());
    }
  }

  return result;
}

} // namespace gxapi::pages::detail
