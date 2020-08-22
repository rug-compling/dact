{ pkgs ? import <nixpkgs> { } }:

let
  sources = import nix/sources.nix;
  dact = pkgs.callPackage ./default.nix { };
  appimage = import "${sources.nix-bundle}/appimage-top.nix" { };
in appimage.appimage (appimage.appdir { name = "dact"; target = dact; })
