{
  description = "C development flake";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = inputs:
    let
      supportedSystems =
        [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forEachSupportedSystem = f:
        inputs.nixpkgs.lib.genAttrs supportedSystems
        (system: f { pkgs = import inputs.nixpkgs { inherit system; }; });

      collections-c = { pkgs }:
        pkgs.stdenv.mkDerivation rec {
          pname = "collections-c";
          version = "unstable-2025-07-13";

          src = pkgs.fetchFromGitHub {
            owner = "srdja";
            repo = "Collections-C";
            rev = "3920f28431ecf82c9e7e78bbcb60fe473d87edf9";
            sha256 = "sha256-rN49u9rWrJFk6xloyFHUaHQjHK8dhiEhGdavBHPXth4=";
          };
          nativeBuildInputs = [ pkgs.cmake pkgs.pkg-config ];
          buildInputs = [ ];
          cmakeFlags = [ "-DSHARED=True" ];
          meta = with pkgs.lib; {
            description = "A library of generic data structures for C";
            homepage = "https://github.com/srdja/Collections-C";
            license = licenses.lgpl3;
            platforms = platforms.all;
          };
        };
    in {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell.override { } {
          packages = with pkgs;
            [
              # c libraries in nixpkgs
              libmicrohttpd
              sqlite

              # Development tools
              clang-tools
              astyle
              cmake
              codespell
              conan
              cppcheck
              doxygen
              gtest
              lcov
              vcpkg
              vcpkg-tool

              # Add Collections-C
              (collections-c { inherit pkgs; })
            ] ++ (if system == "aarch64-darwin" then [ ] else [ gdb ]);
        };
      });
    };
}
