let
  sources = import nix/sources.nix;
  dact = import ./.;
  appimage = import "${sources.nix-bundle}/appimage-top.nix" { };
in appimage.appimage (appimage.appdir { name = "dact"; target = dact.defaultPackage.${builtins.currentSystem}; })
