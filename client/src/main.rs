use std::io::{Read, Write};
use std::net::{TcpStream};
use std::thread;
use std::time::{Duration};
use rand::{thread_rng, Rng};

use structopt::StructOpt;

#[derive(StructOpt, Debug)]
#[structopt(name = "socket_client")]
struct Opt {
    /// Port number to connect to
    #[structopt(short, long, default_value = "9090")]
    port: u16,
    #[structopt(short, long, help = "Interval between reads in milliseconds.", default_value = "0")]
    read_interval: u64,
    #[structopt(short, long, default_value = "100")]
    full_batch: u64,
    #[structopt(short="P", long, default_value = "2")]
    partial_batch: u64,
}

fn main() {
    let opt = Opt::from_args();

    let message = "Hello World, UIC CS463 was here!\n";
    let addr = format!("localhost:{}", opt.port);

    // Connect to the server
    match TcpStream::connect(addr) {
        Ok(mut stream) => {
            println!("Successfully connected to server in port 9090");

            // Split the TcpStream for simultaneous read and write
            let mut reader = stream.try_clone().expect("clone failed...");
            
            let _handle = thread::spawn(move || {
                let mut received_data = String::new();
                loop {
                    let mut buffer = [0; 512];
                    match reader.read(&mut buffer) {
                        Ok(size) => {
                            // Exit if connection is closed
                            if size == 0 {
                                break;
                            }
                            received_data.push_str(&String::from_utf8_lossy(&buffer[..size]));
                            
                            // Process each complete message received
                            let mut end_index;
                            while {
                                // Find the index of the end of the first complete message
                                end_index = received_data.find('\n');
                                end_index.is_some()
                            } {
                                if let Some(index) = end_index {
                                    // Extract the message from the buffer
                                    let line = &received_data[..=index];
                                    // Verify the extracted message
                                    if line.trim() != message.trim() {
                                        println!("Received an incorrect message: {}", line.trim());
                                    }
                                    // Remove the processed message from the buffer
                                    received_data = received_data[index + 1..].to_string();
                                    std::thread::sleep(std::time::Duration::from_millis(opt.read_interval));
                                }
                            }
                        },
                        Err(e) => {
                            println!("Failed to receive data: {}", e);
                            break;
                        }
                    }
                }
            });
            
            loop { 
                // Send some messages one after another
                for _ in 0..opt.full_batch {
                    stream.write_all(message.as_bytes()).expect("write failed");
                    stream.flush().expect("flush failed");
                }

                // Send some messages in random-sized chunks
                for _ in 0..opt.partial_batch {
                    let mut msg_bytes = message.as_bytes();
                    while !msg_bytes.is_empty() {
                        // Choose a random chunk size
                        let chunk_size = thread_rng().gen_range(1..=msg_bytes.len());
                        stream.write_all(&msg_bytes[..chunk_size]).expect("write failed");
                        stream.flush().expect("flush failed");
                        msg_bytes = &msg_bytes[chunk_size..];

                        // Sleep for a random duration between 0 and 1,000,000 microseconds
                        let sleep_duration = thread_rng().gen_range(0..1000000);
                        thread::sleep(Duration::from_micros(sleep_duration));
                    }
                }
            }
        },
        Err(e) => {
            println!("Failed to connect: {}", e);
        }
    }
}
