{
  description = "UHMI test utility";

  nixConfig.bash-prompt = "[\\033[1;36mnix-develop \\033[1;33muhmitest\\033[0m]$ ";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    nix-vscode-extensions = {
      url = "github:nix-community/nix-vscode-extensions";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    nix-vscode-extensions,
  }: let
    system = "x86_64-linux";

    pkgs = nixpkgs.legacyPackages.${system};

    extensions = nix-vscode-extensions.extensions.${system};

    inherit (pkgs) vscode-with-extensions vscodium;

    vscode = vscode-with-extensions.override {
      vscode = vscodium;
      vscodeExtensions = [
        extensions.vscode-marketplace.ms-vscode.cpptools
        extensions.vscode-marketplace.github.vscode-github-actions
        extensions.vscode-marketplace.bbenoist.nix
      ];
    };

  in {

    packages.${system} = rec {
      uhmitest = pkgs.callPackage ./default.nix {};
      default = uhmitest;
    };

    devShells.${system} = rec {
      uhmitest = pkgs.mkShell {
        packages = [vscode];
        inputsFrom = builtins.attrValues self.packages.${system};
        # shellHook = ''
        #     printf "LGnames: ${GLnames}\n"
        # '';
      };
      default = uhmitest;
    };
  };
}
