open System
open System.IO
open System.IO.Ports

// use "dotnet run" from this folder to build and run project

let ports = (SerialPort.GetPortNames ())
for p in ports do
  printfn "ports: %A" p

// manually change this to the port the pico is on
let portname = "COM4"

let port = new SerialPort (portname, 115200, Parity.None, 8, StopBits.One)

port.Open ()
port.DtrEnable   <- true
port.RtsEnable   <- true
port.ReadTimeout <- 3000
//port.WriteTimeout <- 3000

while true do
  try
    let message = (port.ReadLine ())
    Console.WriteLine message
    port.WriteLine "picoack"
    Console.WriteLine "picoack"
    let message = (port.ReadLine ())
    Console.WriteLine message
    port.WriteLine "test"
    Console.WriteLine "test"
    let message = (port.ReadLine ())
    Console.WriteLine message
    File.WriteAllText ("hello.txt", message)
  with
    _ -> Console.WriteLine "no message"

port.Close ()
printfn "program ended"