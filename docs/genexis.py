import info
from Blueprints.CraftPackageObject import CraftPackageObject

class subinfo(info.infoclass):
    def setTargets(self):
        self.displayName = "Genexis"
        self.description = "Cross-platform RGH/JTAG Toolkit"
        self.webpage = "https://genexis.github.io"
        self.svnTargets["main"] = "https://github.com/ExposureMG/Genexis.git|main"
        self.defaultTarget = "main"

    def setDependencies(self):
        self.buildDependencies["virtual/base"] = None
        self.buildDependencies["kde/frameworks/extra-cmake-modules"] = None
        self.runtimeDependencies["libs/zlib"] = None
        self.runtimeDependencies["libs/qt/qtbase"] = None
        self.runtimeDependencies["libs/qt/qtdeclarative"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kirigami"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kcoreaddons"] = None
        self.runtimeDependencies["kde/frameworks/tier1/ki18n"] = None
        self.runtimeDependencies["kde/frameworks/tier1/breeze-icons"] = None
        self.runtimeDependencies["kde/frameworks/tier3/kiconthemes"] = None
        self.runtimeDependencies["kde/frameworks/tier3/qqc2-desktop-style"] = None
        self.runtimeDependencies["kde/plasma/breeze"] = None


class Package(CraftPackageObject.get("kde").pattern):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.subinfo.options.fetch.checkoutSubmodules = True

    def createPackage(self):
        self.defines["executable"] = r"bin\genexis.exe"
        self.addExecutableFilter(r"bin/(?!(genexis)).*")
        return super().createPackage()
