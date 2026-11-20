#include "pages/BadUpdate.hpp"

#include <QJSEngine>

namespace genexis::pages::badupdate {

BadUpdateController::BadUpdateController(QObject *parent) : QObject(parent) {}

BadUpdateController &BadUpdateController::instance() {
    static BadUpdateController inst;
    return inst;
}

BadUpdateController *BadUpdateController::create(QQmlEngine *, QJSEngine *) {
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

} // namespace genexis::pages::badupdate
