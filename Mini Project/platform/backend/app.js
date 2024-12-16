const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const { spawn } = require("child_process");

const app = express();
app.use(bodyParser.json());
app.use(cors());

// Endpoint to calculate the best path
app.post("/api/best-path", (req, res) => {

    const { matrixSize , numSalespeople , distanceMatrix } = req.body;

    if (!Array.isArray(distanceMatrix) || distanceMatrix.length === 0) {
      return res.status(400).json({ error: "Invalid distanceMatrix input" });
    }

    // Convert matrix to string for C++ program
    const input = `${matrixSize} ${numSalespeople}\n` + distanceMatrix.map(row => row.join(" ")).join("\n");

    // Spawn the C++ process
    const cppProcess = spawn("../../genetic_algorithm");

    let output = "";
    let errorOutput = ""; 
    cppProcess.stdout.on("data", (data) => {
      output += data.toString();
    });

    
    cppProcess.stderr.on("data", (data) => {
      console.error("C++ Error:", data.toString());
    });

    cppProcess.on("close", (code) => {
      if (code !== 0) {
        console.error("C++ program failed with exit code:", code);
        return res.status(500).json({ error: "C++ program failed to execute" });
      }

      if (!output || !output.trim()) {
          console.error("C++ program returned no output or invalid output");
          return res.status(500).json({ error: "No valid output from C++ program" });
      }

      try {
        // Parse the output

        // console.log("Raw C++ Output:", output); // Log for debugging

        const lines = output.trim().split("\n");
        const totalDistanceLine = lines[0]?.split(":")[1]?.trim();
        const individualDistancesLine = lines[1]?.split(":")[1]?.trim();

        if (!totalDistanceLine || !individualDistancesLine) {
            console.error("Output format is invalid or missing key values");
            return res.status(500).json({ error: "Invalid output format from C++ program" });
        }

        const salespersonVisits = lines.slice(2).map(line => {
          const [salesperson, visits] = line.split("->").map(s => s.trim());
          return {
            salesperson: parseInt(salesperson.match(/\d+/)[0], 10), // Extract salesperson index
            visits: visits.split(" ").map(Number), // Parse visits as an array of integers
          };
        });

        const response = {
          totalDistance: parseFloat(totalDistanceLine),
          individualDistances: individualDistancesLine.split(" ").map(parseFloat),
          salespersonVisits,
        };

        // Send the structured response to the frontend
        console.log("sending result !"); 
        res.json(response);
      } catch (err) {
        console.error("Error parsing C++ output:", err.message);
        return res.status(500).json({ error: "Failed to process C++ program output" });
      }
    });

  // Send the distance matrix to the C++ program
  cppProcess.stdin.write(input);
  cppProcess.stdin.end();

  console.log("result sent !"); 
 
});

app.listen(5000, () => console.log("Server running on http://localhost:5000"));
