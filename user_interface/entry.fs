open System
open System.IO
open System.IO.Ports

// use "dotnet run" from this folder to build and run project

(*let pico_repl (portName: string) =
  while true do
  *)  

let ports = (SerialPort.GetPortNames ())
for p in ports do
  printfn "ports: %A" p

// manually change this to the port the pico is on
let portname = ports.[0]

let port = new SerialPort (portname, 115200, Parity.None, 8, StopBits.One)

port.Open ()
port.DtrEnable   <- true
port.RtsEnable   <- true
port.ReadTimeout <- 3000
//port.WriteTimeout <- 3000

let port_command cmd fn =
  port.WriteLine cmd
  Console.WriteLine cmd
  let message = (port.ReadLine ())
  Console.WriteLine message
  File.WriteAllText (fn, message)

while true do
  try
    let message = (port.ReadLine ())
    Console.WriteLine message
    port.WriteLine "picoack"
    Console.WriteLine "picoack"
    let message = (port.ReadLine ())
    Console.WriteLine message
    port_command "test" "output/hello.txt"
    port_command "spi_test" "output/hello_spi.txt"
    port_command "i2c_test" "output/hello_i2c.txt"
    port_command "adc_test" "output/hello_adc.txt"
  with
    _ -> Console.WriteLine "no message"

port.Close ()
printfn "program ended"