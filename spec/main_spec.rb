describe 'gameboy' do
  def run_emu_dbg(argv,commands)
    raw_output = nil
    IO.popen(argv, "r+") do |pipe|
      commands.each do |command|
        pipe.puts command
      end

      pipe.close_write

      # Read entire output
      raw_output = pipe.gets(nil)
    end
    raw_output.split("\n")
  end

  def run_emu(argv)
    output = IO.popen(argv, "r+", :err=>[:child, :out]).read
    output.split("\n")
  end

  describe 'blarggs test roms' do
    # Got the address of the breakpoint by running interactively
    # until test rom completed. cpu_state log file has last instr
    it 'cpu_instr test rom' do
      argv = [
        "bin/GBcon",
        "--bios", "tests/resources/gb_bios.bin",
        "--rom", "tests/resources/blarggs/cpu_instrs.gb",
        "--log", "log",
        "--dbg",
      ]
      debugger_commands = [
        "break 0x06f1",
        "continue",
        "quit",
      ]
      run_emu_dbg(argv,debugger_commands)
      result = File.readlines("log/serial.log").each{|line| line.strip!}
      expect(result).to match_array([
          "cpu_instrs",
          "",
          "01:ok  02:04  03:ok  04:ok  05:ok  "\
          "06:ok  07:ok  08:ok  09:ok  10:ok  11:ok",
          "",
          "Failed 1 tests.",
      ])
    end
  end

  describe 'command line arguement parsing' do
    it 'prints error when --rom arg missing' do
      argv = [
        "bin/GBcon",
        "--bios", "tests/resources/gb_bios.bin",
      ]
      result = run_emu(argv)
      expect(result).to match_array([
        "the option '--rom' is required but missing"
      ])
    end
  end

  describe 'input files do not exit' do
    it 'prints error when rom file does not exist' do
      argv = [
        "bin/GBcon",
        "--bios", "tests/resources/gb_bios.bin",
        "--rom", "NOT_A_FILE",
      ]
      result = run_emu(argv)
      expect(result).to match_array([
        "GBcon: Error reading rom file"
      ])
    end
  end
end
