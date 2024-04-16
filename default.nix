{ stdenv, kernel }:
let
  version = "0.1.0";
in
stdenv.mkDerivation rec {
  name = "klog-${version}-${kernel.version}";
  inherit version;

  src = ./src;
  installPhase = ''
      mkdir $out
      cp -r * $out/
  '';

  hardeningDisable = [ "pic" "format" ];
  nativeBuildInputs = kernel.moduleBuildDependencies;
  makeFlags = [
    "KERNELRELEASE=${kernel.modDirVersion}"
    "KERNEL_DIR=${kernel.dev}/lib/modules/${kernel.modDirVersion}/build"
    "INSTALL_MOD_PATH=$(out)"
  ];
}
