{
  pkgs ? import <nixpkgs> {}
}:

with pkgs;

let
  sources = import nix/sources.nix;
  alpinocorpus = callPackage sources.alpinocorpus {};
in qt5.mkDerivation {
  name = "dact";

  src = nix-gitignore.gitignoreSource [] ./.; 

  nativeBuildInputs = [
    cmake
    git
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
