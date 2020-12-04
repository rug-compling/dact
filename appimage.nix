let
  nix-bundle = fetchTarball {
    url = "https://github.com/matthewbauer/nix-bundle/archive/27f20492c5ac45ffbf2540aaa97a69d9e911249b.tar.gz";
    sha256 = "13lm6mdhx3shri69c208iv5yqr94ija4nzy220ag3jlg9bmjl8gz";
  };
  flake = import ./.;
  dact = flake.default;
  appimage = import "${nix-bundle}/appimage-top.nix" { nixpkgs' = flake.inputs.nixpkgs; };
in appimage.appimage (appimage.appdir { name = "dact"; target = dact; })
