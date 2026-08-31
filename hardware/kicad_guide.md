# KiCad Build Guide — EnvSense Board

This is the part you should do yourself in the KiCad GUI — it's where the actual
"PCB design" learning happens, and it's what interviewers will ask you about.
This guide gets you from a blank project to gerbers ready to send to a fab.
Budget **6-10 hours** spread over a few sessions for your first board.

Install KiCad 8 (free) from https://www.kicad.org before starting.

## 1. Create the project

1. Open KiCad, click **New Project**, name it `envsense`, save it inside
   this `hardware/` folder.
2. This creates a `.kicad_pro` file — that's your project file. Commit it
   to git along with everything KiCad generates from here on.

## 2. Schematic capture (Eeschema)

1. Open the Schematic Editor.
2. Place symbols using `A` (add symbol). Search for and place:
   - `MCU_ST_STM32F1:STM32F103C8Tx` (the STM32)
   - `Sensor:BME280` (search "BME280" — if it's not in the default library,
     use a generic 8-pin part and label pins manually per the connection table)
   - `Regulator_Linear:AMS1117-3.3`
   - `Device:C` for every capacitor, `Device:R` for every resistor
   - `Device:Crystal` for Y1
   - `Switch:SW_Push` for the reset/boot buttons
   - `Connector_Generic:Conn_01x04` for your headers (SWD, UART, I2C)
   - `Device:LED` for the two status LEDs
3. Wire everything up using the connection table in `connection_table.md`.
   Use **labels** (press `L`) instead of long wires for nets like `3V3`,
   `GND`, `SCL`, `SDA` — it keeps the schematic readable.
4. Add power symbols (`GLabel` or dedicated `power:+3V3` / `power:GND` symbols)
   so ERC (electrical rules check) can verify every net has a driver.
5. Run **Inspect > Electrical Rules Checker**. Fix every error before moving
   on — this is the single most valuable habit to build now. Warnings about
   unconnected pins on the STM32 you're not using are fine to leave.
6. Assign footprints: **Tools > Assign Footprints**. For example:
   - STM32F103C8T6 -> `Package_QFP:LQFP-48_7x7mm_P0.5mm`
   - Resistors/caps -> `Resistor_SMD:R_0603_1608Metric` / `Capacitor_SMD:C_0603_1608Metric`
   - Crystal -> whatever THT/SMD package matches what you actually bought

## 3. PCB layout (PCB Editor)

1. Open the PCB Editor, click **Update PCB from Schematic** (or press F8).
   All your parts appear as a pile of footprints outside the board outline.
2. Draw a board outline on the **Edge.Cuts** layer — start simple, a
   40mm x 60mm rectangle is plenty for this design.
3. Arrange parts logically before routing:
   - STM32 roughly centered
   - Crystal (Y1) close to the STM32's OSC pins — keep these traces *short*
   - Decoupling caps directly adjacent to their MCU VDD pins
   - BME280 near an edge, away from heat sources, ideally with a small
     cutout/slot nearby if you want it exposed to ambient air later
   - Headers (SWD, UART, I2C) along one edge for easy access
4. Set your track width and via size (**Route > Design Rules**) — 0.25mm
   traces / 0.6mm vias is a safe default for a 2-layer hobby board.
5. Route traces: press `X` or use the route tool. Route power (3.3V, GND)
   first, then signals. For a 2-layer board, it's fine to route mostly on
   the top copper layer and use the bottom layer as a rough ground pour.
6. Add a **ground pour** on the bottom layer: **Add Filled Zone**, select
   `B.Cu`, assign it to the GND net, right-click to fill.
7. Run **Inspect > Design Rules Checker (DRC)**. Fix every error — no
   exceptions. Unrouted nets show up here too.
8. Add silkscreen labels for your headers (SWD pin 1, UART TX/RX, etc.) —
   future-you (and interviewers looking at your GitHub photos) will thank you.

## 4. Generate manufacturing files

1. **File > Fabrication Outputs > Gerbers** — export with default layers
   (F.Cu, B.Cu, F.SilkS, B.SilkS, F.Mask, B.Mask, Edge.Cuts).
2. **File > Fabrication Outputs > Drill Files**.
3. Zip the gerber + drill files — this is what you upload to a fab like
   JLCPCB or PCBWay (~$5-15 for 5 boards, few days shipping).
4. Also export a BOM (**Tools > Generate BOM**) and a placement/position
   file if you want the fab to assemble SMD parts for you (optional, and
   costs more — hand-soldering this board is very doable).

## 5. Commit everything to git

Add to your repo:
- `envsense.kicad_pro`, `envsense.kicad_sch`, `envsense.kicad_pcb`
- The `gerbers/` folder (zipped or unzipped)
- Exported schematic PDF (**File > Plot > PDF** in Eeschema) — this is
  what most reviewers will actually look at, since it renders directly on
  GitHub
- A screenshot or two of the 3D view (**View > 3D Viewer** in PCB editor,
  then File > Export > Current View as PNG) for your README

## Tips for a smooth first board

- **Route power and ground before anything else.** Everything else is
  easier once those are solid.
- **Don't be afraid to leave headroom** on the board outline — cramped
  layouts are much harder to route cleanly as a beginner.
- If DRC is throwing errors you don't understand, that's completely
  normal for a first board — search the exact error text, KiCad's
  community forum and docs are excellent.
- Order 5 boards minimum (most fabs' minimum anyway) — you will make at
  least one mistake on v1, and having spares to try fixes on (bodge wires,
  rework) is worth it.
