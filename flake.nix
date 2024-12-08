{
  description = "rf_pi: send rf codes from your Pi";
  # inputs.nixpkgs.url = "nixpkgs/release-23.05";
  # inputs.nixpkgs.url = "github:nixos/nixpkgs/release-23.05";
  inputs.nixpkgs.url = "github:nixos/nixpkgs";

  outputs =
    {
      self,
      nixpkgs,
    }:
    let
      version = builtins.substring 0 8 self.lastModifiedDate;
      supportedSystems = [
        # "x86_64-linux"
        # "x86_64-darwin"
        "aarch64-linux"
        # "aarch64-darwin"
      ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      nixpkgsFor = forAllSystems (
        system:
        import nixpkgs {
          inherit system;
          overlays = [ self.overlay ];
        }
      );
      pname = "rf_pi";
    in
    {
      overlay = final: _: {
        ${pname} =
          let
            inherit (final) symlinkJoin python3Packages writers;
            rf_send_lib = final.gcc9Stdenv.mkDerivation {
              inherit pname;
              inherit version;

              src = ./.;

              nativeBuildInputs = with final; [
                sudo
                libcap
                wiringpi
              ];

              installPhase = ''
                install -D -t $out/lib *.so
                install -D -t $out/bin print_sched RFSniffer send
              '';
            };
          in
          symlinkJoin {
            name = pname;
            paths = [
              (
                (writers.writePython3Bin "rf_send.py" { libraries = [ python3Packages.rpi-gpio ]; } (
                  builtins.readFile ./rf_send.py
                )).overrideAttrs
                (old: {
                  buildCommand =
                    old.buildCommand
                    + ''
                      substituteInPlace $out/bin/rf_send.py \
                        --replace-fail "rf_lib = os.path.join(dirname, 'send.so')" 'rf_lib = "${rf_send_lib}/lib/send.so"'
                    '';
                })
              )
            ];
          };
      };

      packages = forAllSystems (system: {
        inherit (nixpkgsFor.${system}) rf_pi;
        default = self.outputs.packages.${system}.rf_pi;
      });

      apps = forAllSystems (system: {
        default = self.outputs.apps.${system}.${pname};
        ${pname} = {
          type = "app";
          program = "${self.outputs.packages.${system}.rf_pi}/bin/rf_send.py";
        };
      });
    };
}
