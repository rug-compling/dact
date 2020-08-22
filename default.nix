{
  pkgs ? import <nixpkgs> {}
, alpinocorpus ? pkgs.callPackage (import nix/sources.nix).alpinocorpus {}
}:

with pkgs;

qt5.mkDerivation {
  name = "dact";

  src = nix-gitignore.gitignoreSource [ ".git" "*.nix" "flake.lock" ] ./.;

  nativeBuildInputs = [
    git
    meson
    ninja
    pkgconfig
  ];

  buildInputs = [
    alpinocorpus
    boost
    libxml2
    libxslt
    qt5.qtbase
    xercesc
    xqilla
    zlib
  ];
} 
