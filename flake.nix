{
  description = "Dact treebank search & evaluation tool";

  inputs = {
    alpinocorpus = {
      url = "github:rug-compling/alpinocorpus";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        utils.follows = "utils";
      };
    };
    nixpkgs.url = "github:NixOS/nixpkgs/release-20.09";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, alpinocorpus, nixpkgs, utils }:
    utils.lib.eachSystem [ "aarch64-linux" "x86_64-linux" ] (system: {
      defaultPackage = self.packages.${system}.dact;

      packages.dact = with nixpkgs.legacyPackages.${system}; qt5.mkDerivation {
        pname = "dact";
        version = "3.0.0";

        src = ./.;

        nativeBuildInputs = [
          git
          meson
          ninja
          pkg-config
        ];

        buildInputs = [
          alpinocorpus.packages.${system}.alpinocorpus
          boost
          libxml2
          libxslt
          qt5.qtbase
          xercesc
          xqilla
          zlib
        ];

        meta = with lib; {
          description = "Decaffeinated Alpino Corpus Tool";
          homepage = "https://github.com/rug-compling/dact";
          license = licenses.lgpl21;
          platforms = platforms.unix;
        };
      };
    });
}
