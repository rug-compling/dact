with import <nixpkgs> {};
stdenv.mkDerivation rec {
  name = "alpinocorpus-env";
  env = buildEnv { name = name; paths = buildInputs; };

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
