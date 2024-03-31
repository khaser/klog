{
  description = "Environment to develop linux out-of-tree module";

  inputs = {
    nixpkgs.follows = "khaser/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    khaser.url = "git+ssh://git@109.124.253.149/~git/nixos-config?ref=master";
  };

  outputs = { self, nixpkgs, flake-utils, khaser }:
    flake-utils.lib.eachDefaultSystem ( system:
    let
      pkgs = import nixpkgs { inherit system; };
      kernel = pkgs.linuxKernel.kernels.linux_6_1;
      configured-vim = khaser.lib.vim.override {
        extraRC = ''
          let &path.="${kernel.dev}/lib/modules/6.1.68/build/source/include"
          set colorcolumn=81
        '';
      };
      klog-module = (pkgs.stdenv.mkDerivation rec {
          name = "klog-${version}-${kernel.version}";
          version = "0.0.1";

          src = ./src;
          installPhase = ''
            mkdir $out
            cp -r * $out/
          '';

          hardeningDisable = [ "pic" "format" ];
          nativeBuildInputs = kernel.moduleBuildDependencies;
          makeFlags = [
            "KERNELRELEASE=${kernel.modDirVersion}"                                 # 3
            "KERNEL_DIR=${kernel.dev}/lib/modules/${kernel.modDirVersion}/build"    # 4
            "INSTALL_MOD_PATH=$(out)"                                               # 5
          ];
        });
    in {
      packages.default = klog-module;

      devShell = pkgs.mkShell {
        name = "linux-klog";

        nativeBuildInputs = with pkgs; [
          gcc # compiler
          configured-vim
        ];

      };
    });
}

