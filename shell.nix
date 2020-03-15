with import <nixpkgs> {};

let
  dact = callPackage ./default.nix {};
in mkShell {
  inherit (dact) nativeBuildInputs buildInputs;

  QT_PLUGIN_PATH = "${qt5.qtbase}/${qt5.qtbase.qtPluginPrefix}";
}
