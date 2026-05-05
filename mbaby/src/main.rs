use clap::{ArgAction, ArgGroup, Parser};
use std::{
    fmt,
    io::{BufRead, Write},
    path::Path,
    process::ExitCode,
};

const MEMORY_SIZE: usize = 32;
const WORD_BITS: usize = 32;
const ADDR_MASK: i32 = 0x1F;
const OPCODE_SHIFT: u32 = 13;
const OPCODES: [&str; 8] = ["JMP", "JRP", "LDN", "STO", "SUB", "SUB", "CMP", "STP"];

#[derive(Parser, Debug)]
#[command(group(
    ArgGroup::new("input")
        .required(true)
        .args(["file_ass", "file_snp"]),
))]
struct Args {
    /// Increase output detail (-v, -vv, -vvv)
    #[arg(short = 'v', long = "verbose", action = ArgAction::Count)]
    verbose: u8,

    /// Load input program as mnemonic assembly and output a memory snapshot unless --run is selected
    #[arg(short = 's', long = "assembly", value_name = "FILE")]
    file_ass: Option<String>,

    /// Load input program as a memory snapshot and disassemble unless --run is selected
    #[arg(short = 'm', long = "memory-snapshot", value_name = "FILE")]
    file_snp: Option<String>,

    /// Execute the loaded program
    #[arg(short = 'r', long = "run")]
    run: bool,

    /// Stop execution after N instructions instead of running forever
    #[arg(long = "max-steps", value_name = "N")]
    max_steps: Option<usize>,
}

#[derive(Debug)]
struct DecodedInstruction {
    opcode: usize,
    mnemonic: &'static str,
    addr: usize,
}

#[derive(Debug)]
struct ExecutionReport {
    memory: [i32; MEMORY_SIZE],
    ci: usize,
    accumulator_a: i32,
    steps: usize,
}

fn main() -> ExitCode {
    match run(Args::parse()) {
        Ok(()) => ExitCode::SUCCESS,
        Err(err) => {
            eprintln!("Error: {err}");
            ExitCode::FAILURE
        }
    }
}

fn run(args: Args) -> Result<(), String> {
    if args.max_steps == Some(0) {
        return Err("--max-steps must be greater than 0".to_string());
    }

    match (&args.file_ass, &args.file_snp) {
        (Some(asm_filename), None) => {
            verbose_log(
                args.verbose,
                1,
                format_args!("Loading assembly from {asm_filename}"),
            );
            let memory = load_mem_from_asm(asm_filename, args.verbose)?;
            if args.run {
                let report = run_baby_run(&memory, args.verbose, args.max_steps)?;
                print_execution_report(&report, args.verbose);
            } else {
                print_memory_snapshot(&memory, args.verbose);
            }
        }

        (None, Some(snp_filename)) => {
            verbose_log(
                args.verbose,
                1,
                format_args!("Loading memory snapshot from {snp_filename}"),
            );
            let memory = load_mem_from_snapshot(snp_filename, args.verbose)?;
            if args.run {
                let report = run_baby_run(&memory, args.verbose, args.max_steps)?;
                print_execution_report(&report, args.verbose);
            } else {
                disassembly(&memory, args.verbose);
            }
        }

        _ => unreachable!("clap requires exactly one input source"),
    }

    Ok(())
}

fn load_mem_from_asm(
    filename: impl AsRef<Path>,
    verbose: u8,
) -> Result<[i32; MEMORY_SIZE], String> {
    let filename = filename.as_ref();
    let mut memory = [0; MEMORY_SIZE];
    let mut written = [false; MEMORY_SIZE];
    let asm_file = std::fs::File::open(filename).map_err(|err| {
        format!(
            "could not open assembly file '{}': {err}",
            filename.display()
        )
    })?;
    let reader = std::io::BufReader::new(asm_file);
    let mut loaded_lines = 0usize;

    for (line_idx, line) in reader.lines().enumerate() {
        let line_no = line_idx + 1;
        let line = line.map_err(|err| {
            format!(
                "{}:{line_no}: could not read assembly line: {err}",
                filename.display()
            )
        })?;
        let code = strip_comment(&line).trim();
        if code.is_empty() {
            continue;
        }

        let normalized = code.replace(':', " ");
        let instr = normalized.split_whitespace().collect::<Vec<_>>();
        if instr.len() < 2 {
            return Err(format!(
                "{}:{line_no}: expected '<address> <mnemonic> [operand]'",
                filename.display()
            ));
        }
        if instr.len() > 3 {
            return Err(format!(
                "{}:{line_no}: unexpected extra token '{}'",
                filename.display(),
                instr[3]
            ));
        }

        let mem_addr = parse_memory_address(instr[0], filename, line_no)?;
        if written[mem_addr] {
            verbose_log(
                verbose,
                1,
                format_args!(
                    "{}:{line_no}: overwriting memory address {mem_addr}",
                    filename.display()
                ),
            );
        }

        let mnemonic = instr[1].to_ascii_uppercase();
        let operand = instr
            .get(2)
            .map(|value| parse_i32_operand(value, filename, line_no))
            .transpose()?;
        memory[mem_addr] = assemble_word(&mnemonic, operand, filename, line_no)?;
        written[mem_addr] = true;
        loaded_lines += 1;

        verbose_log(
            verbose,
            2,
            format_args!(
                "{}:{line_no}: assembled {:2} -> {}",
                filename.display(),
                mem_addr,
                to_binary_string(memory[mem_addr])
            ),
        );
    }

    verbose_log(
        verbose,
        1,
        format_args!(
            "Loaded {loaded_lines} assembly line(s) into {} words of memory",
            MEMORY_SIZE
        ),
    );
    Ok(memory)
}

fn load_mem_from_snapshot(
    filename: impl AsRef<Path>,
    verbose: u8,
) -> Result<[i32; MEMORY_SIZE], String> {
    let filename = filename.as_ref();
    let mut memory = [0; MEMORY_SIZE];
    let snapshot_file = std::fs::File::open(filename).map_err(|err| {
        format!(
            "could not open snapshot file '{}': {err}",
            filename.display()
        )
    })?;
    let reader = std::io::BufReader::new(snapshot_file);
    let mut next_addr = 0usize;

    for (line_idx, line) in reader.lines().enumerate() {
        let line_no = line_idx + 1;
        let line = line.map_err(|err| {
            format!(
                "{}:{line_no}: could not read snapshot line: {err}",
                filename.display()
            )
        })?;
        let code = strip_comment(&line).trim();
        if code.is_empty() {
            continue;
        }
        if next_addr >= MEMORY_SIZE {
            return Err(format!(
                "{}:{line_no}: snapshot contains more than {MEMORY_SIZE} words",
                filename.display()
            ));
        }

        let word_text = code
            .rsplit_once(':')
            .map_or(code, |(_, instruction)| instruction)
            .trim();
        if word_text.split_whitespace().count() != 1 {
            return Err(format!(
                "{}:{line_no}: expected one binary word",
                filename.display()
            ));
        }

        memory[next_addr] = to_int(word_text).map_err(|err| {
            format!(
                "{}:{line_no}: invalid binary word '{}': {err}",
                filename.display(),
                word_text
            )
        })?;
        verbose_log(
            verbose,
            2,
            format_args!(
                "{}:{line_no}: loaded {:2} <- {}",
                filename.display(),
                next_addr,
                word_text
            ),
        );
        next_addr += 1;
    }

    if next_addr < MEMORY_SIZE {
        verbose_log(
            verbose,
            1,
            format_args!(
                "Snapshot supplied {next_addr} word(s); remaining {} word(s) left as zero",
                MEMORY_SIZE - next_addr
            ),
        );
    }

    Ok(memory)
}

fn assemble_word(
    mnemonic: &str,
    operand: Option<i32>,
    filename: &Path,
    line_no: usize,
) -> Result<i32, String> {
    let context = || format!("{}:{line_no}", filename.display());

    match mnemonic {
        "JMP" | "JRP" | "LDN" | "STO" | "SUB" => {
            let addr =
                operand.ok_or_else(|| format!("{}: {mnemonic} requires an address", context()))?;
            let opcode = opcode_for(mnemonic).expect("mnemonic should have an opcode");
            Ok((opcode << OPCODE_SHIFT) | (addr & ADDR_MASK))
        }
        "CMP" | "STP" => {
            if operand.is_some() {
                return Err(format!(
                    "{}: {mnemonic} does not take an address",
                    context()
                ));
            }
            let opcode = opcode_for(mnemonic).expect("mnemonic should have an opcode");
            Ok(opcode << OPCODE_SHIFT)
        }
        "NUM" => operand.ok_or_else(|| format!("{}: NUM requires a value", context())),
        _ => Err(format!("{}: invalid opcode '{mnemonic}'", context())),
    }
}

fn opcode_for(mnemonic: &str) -> Option<i32> {
    match mnemonic {
        "JMP" => Some(0),
        "JRP" => Some(1),
        "LDN" => Some(2),
        "STO" => Some(3),
        "SUB" => Some(4),
        "CMP" => Some(6),
        "STP" => Some(7),
        _ => None,
    }
}

fn parse_memory_address(value: &str, filename: &Path, line_no: usize) -> Result<usize, String> {
    let mem_addr = value.parse::<usize>().map_err(|err| {
        format!(
            "{}:{line_no}: invalid memory address '{}': {err}",
            filename.display(),
            value
        )
    })?;

    if mem_addr >= MEMORY_SIZE {
        return Err(format!(
            "{}:{line_no}: memory address {mem_addr} is out of bounds; expected 0..{}",
            filename.display(),
            MEMORY_SIZE - 1
        ));
    }

    Ok(mem_addr)
}

fn parse_i32_operand(value: &str, filename: &Path, line_no: usize) -> Result<i32, String> {
    value.parse::<i32>().map_err(|err| {
        format!(
            "{}:{line_no}: invalid operand '{}': {err}",
            filename.display(),
            value
        )
    })
}

fn strip_comment(line: &str) -> &str {
    let comment_start = [line.find('#'), line.find(';')]
        .into_iter()
        .flatten()
        .min()
        .unwrap_or(line.len());
    &line[..comment_start]
}

fn to_int(s: &str) -> Result<i32, String> {
    if s.is_empty() {
        return Err("empty binary word".to_string());
    }
    if s.len() > WORD_BITS {
        return Err(format!("word has more than {WORD_BITS} bits"));
    }
    if let Some(invalid) = s.chars().find(|ch| *ch != '0' && *ch != '1') {
        return Err(format!("unexpected character '{invalid}'"));
    }

    u32::from_str_radix(s.chars().rev().collect::<String>().as_str(), 2)
        .map(|word| word as i32)
        .map_err(|err| err.to_string())
}

fn to_crt_string(n: i32) -> String {
    let zero = ' ';
    let one = '\u{25CF}';
    to_big_endian_binary_string(n)
        .chars()
        .rev()
        .map(|c| if c == '1' { one } else { zero })
        .collect::<String>()
}

fn to_big_endian_binary_string(n: i32) -> String {
    format!("{:032b}", n as u32)
}

fn to_binary_string(n: i32) -> String {
    to_big_endian_binary_string(n)
        .chars()
        .rev()
        .collect::<String>()
}

fn print_memory_snapshot(mem: &[i32; MEMORY_SIZE], verbose: u8) {
    for (i, word) in mem.iter().enumerate() {
        if verbose == 0 {
            println!("{i:2}:{}", to_binary_string(*word));
        } else {
            let decoded = decode_word(*word);
            println!(
                "{i:2}:{}  {:<3} {:2}  {:>11}",
                to_binary_string(*word),
                decoded.mnemonic,
                decoded.addr,
                word
            );
            if verbose >= 2 {
                println!("    {}", to_crt_string(*word));
            }
        }
    }
}

fn disassembly(mem: &[i32; MEMORY_SIZE], verbose: u8) {
    for (i, word) in mem.iter().enumerate() {
        let decoded = decode_word(*word);
        println!(
            "{i:2} {:<3} {:2} \t {} \t {}",
            decoded.mnemonic,
            decoded.addr,
            to_binary_string(*word),
            word
        );
        if verbose >= 2 {
            println!("    {}", to_crt_string(*word));
        }
    }
}

fn dump_state(mem: &[i32; MEMORY_SIZE], ci: usize, accumulator_a: i32) {
    for (i, word) in mem.iter().enumerate() {
        let decoded = decode_word(*word);
        let pin = if i == ci { ">>" } else { "  " };
        println!(
            "{i:2} {} \t {pin}{:<3} \t {:2} \t {}",
            to_crt_string(*word),
            decoded.mnemonic,
            decoded.addr,
            word
        );
    }
    println!("-----------------------------------------");
    println!("Current Instruction (CI): {ci}");
    println!(" Accumulator         (A): {accumulator_a}");
}

fn run_baby_run(
    mem: &[i32; MEMORY_SIZE],
    verbose: u8,
    max_steps: Option<usize>,
) -> Result<ExecutionReport, String> {
    let mut memory = *mem;
    let mut ci: usize = 0;
    let mut accumulator_a: i32 = 0;
    let mut steps = 0usize;
    let live_trace = verbose >= 2;

    if live_trace {
        print!("\x1b[2J\x1b[H");
        print!("\x1b[s");
        flush_stdout();
    }

    loop {
        if let Some(limit) = max_steps
            && steps >= limit
        {
            if live_trace {
                print!("\x1b[u");
            }
            return Err(format!(
                "program did not halt within {limit} step(s); rerun with a higher --max-steps value if that is expected"
            ));
        }

        ci = wrap_to_address((ci as i32).wrapping_add(1));
        let pi = memory[ci];
        let decoded = decode_word(pi);
        steps += 1;

        if live_trace {
            print!("\x1b[u");
            dump_state(&memory, ci, accumulator_a);
            println!(
                "Step {steps}: executing {} {}",
                decoded.mnemonic, decoded.addr
            );
            flush_stdout();
        } else {
            verbose_log(
                verbose,
                1,
                format_args!(
                    "step {steps:>5}: CI={ci:2} {:<3} {:2} A={accumulator_a}",
                    decoded.mnemonic, decoded.addr
                ),
            );
        }

        match decoded.opcode {
            0 => ci = wrap_to_address(memory[decoded.addr]),
            1 => ci = wrap_to_address((ci as i32).wrapping_add(memory[decoded.addr])),
            2 => accumulator_a = 0_i32.wrapping_sub(memory[decoded.addr]),
            3 => memory[decoded.addr] = accumulator_a,
            4 | 5 => accumulator_a = accumulator_a.wrapping_sub(memory[decoded.addr]),
            6 => {
                if accumulator_a < 0 {
                    ci = wrap_to_address((ci as i32).wrapping_add(1));
                }
            }
            7 => break,
            _ => unreachable!("decoded opcode should be in range 0..=7"),
        }
    }

    if live_trace {
        print!("\x1b[u");
        flush_stdout();
    }

    Ok(ExecutionReport {
        memory,
        ci,
        accumulator_a,
        steps,
    })
}

fn decode_word(word: i32) -> DecodedInstruction {
    let opcode = ((word & 0xE000) >> OPCODE_SHIFT) as usize;
    DecodedInstruction {
        opcode,
        mnemonic: OPCODES[opcode],
        addr: (word & ADDR_MASK) as usize,
    }
}

fn wrap_to_address(value: i32) -> usize {
    (value & ADDR_MASK) as usize
}

fn print_execution_report(report: &ExecutionReport, verbose: u8) {
    println!(
        "Stopped after {} step(s). CI={} A={}",
        report.steps, report.ci, report.accumulator_a
    );

    if verbose >= 1 {
        dump_state(&report.memory, report.ci, report.accumulator_a);
    }
}

fn verbose_log(verbose: u8, level: u8, message: fmt::Arguments<'_>) {
    if verbose >= level {
        eprintln!("{message}");
    }
}

fn flush_stdout() {
    std::io::stdout()
        .flush()
        .expect("stdout should be flushable");
}
