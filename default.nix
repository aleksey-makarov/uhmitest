{ stdenv, lib, pkg-config, libdrm, mesa, libGL, cmake } :

stdenv.mkDerivation rec {
    pname = "uhmitest";
    version = "0.1";

    src = ./.;

    buildInputs = [ libdrm mesa libGL ];
    nativeBuildInputs = [ pkg-config cmake ];

    meta = with lib; {
        homepage = "https://www.opensynergy.com/";
        description = "Test for UHMI";
        license = licenses.mit;
        platforms = platforms.unix;
        pkgConfigModules = [ libuhmigl ];
    };
}
