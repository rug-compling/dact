{
  description = "Application for viewing and searching Alpino treebanks";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-20.09";
    alpinocorpus = {
      url = "github:rug-compling/alpinocorpus/flake";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, alpinocorpus, flake-utils, nixpkgs }:
    flake-utils.lib.eachDefaultSystem (system: rec {
      defaultApp = apps.dact;
      defaultPackage = packages.dact;

      apps.dact = {
        program = "${packages.dact}/bin/dact";
        type = "app";
      };

      packages.dact = with nixpkgs.legacyPackages.${system}; qt5.mkDerivation {
        pname = "dact";
        version = "3.0.0";

        src = ./.;

        nativeBuildInputs = [
          git
          meson
          ninja
          pkgconfig
        ];

        buildInputs = [
          alpinocorpus.defaultPackage.${system}
          boost
          libxml2
          libxslt
          qt5.qtbase
          xercesc
          xqilla
          zlib
        ];
      }; 
    });
}
