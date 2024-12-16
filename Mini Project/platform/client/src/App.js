import React, { useState } from "react";
import axios from "axios";
import "./App.css";
import Header from "./Header";

const MAPBOX_TOKEN = "pk.eyJ1IjoiYW11bC00OSIsImEiOiJjbTFkZjA2NHUyazZvMmtzYjhxejJudWZvIn0.igsNvH1LnXYEhwxe16SZMg";

const App = () => {
  const [locations, setLocations] = useState([""]); // Ensure locations starts as an array
  const [distanceMatrix, setDistanceMatrix] = useState([]); // Ensure distanceMatrix starts as an empty array
  const [bestPathResult, setBestPathResult] = useState(null);
  const [numSalespeople, setNumSalespeople] = useState(1); // New state for salespeople count


  // Add new location input
  const handleAddLocation = () => setLocations([...locations, ""]);

  // Update a specific location input
  const handleLocationChange = (index, value) => {
    const updatedLocations = [...locations];
    updatedLocations[index] = value;
    setLocations(updatedLocations);
  };

  // Calculate the distance matrix using Mapbox
  const handleCalculateMatrix = async () => {
    try {
      if (locations.some((location) => location.trim() === "")) {
        alert("Please fill in all location fields.");
        return;
      }
  
      const coordinates = await Promise.all(
        locations.map(async (location) => {
          const geocodeUrl = `https://api.mapbox.com/geocoding/v5/mapbox.places/${encodeURIComponent(
            location
          )}.json?access_token=${MAPBOX_TOKEN}`;
          const { data } = await axios.get(geocodeUrl);
  
          if (!data.features || !data.features.length) {
            console.error(`No results for location: ${location}`);
            return [];
          }
          return data.features[0]?.geometry.coordinates || [];
        })
      );
  
      // Construct URL for the Directions Matrix API
      const matrixUrl = `https://api.mapbox.com/directions-matrix/v1/mapbox/driving/${coordinates
        .filter((c) => c.length === 2)
        .map((c) => c.join(","))
        .join(";")}?annotations=distance&access_token=${MAPBOX_TOKEN}`;
  
      // console.log("Matrix API URL:", matrixUrl); // Debug
  
      const matrixResponse = await axios.get(matrixUrl);

      // Convert distances from meters to kilometers
      const distancesInKm = matrixResponse.data.distances.map((row) =>
        row.map((distance) => distance / 1000) // Convert meters to kilometers
      );
      // console.log(distancesInKm ); 
  
      // Set the distance matrix from the API response
      setDistanceMatrix(distancesInKm  || []);
      // console.log("Matrix distances set:", matrixResponse.data.distances);
    } catch (error) {
      console.error("Error calculating matrix:", error.message);
    }
  };
  
  // Send distance matrix to backend to find the best path
  const handleFindBestPath = async () => {
    try {
      if (!distanceMatrix.length) {
        alert("Please calculate the distance matrix first.");
        return;
      }

      const matrixSize = distanceMatrix.length ; 
      const reqBody = {
        matrixSize , 
        numSalespeople ,
        distanceMatrix  
      } ; 
      const response = await axios.post("http://localhost:5000/api/best-path", reqBody);
      // console.log(response.data); 
      setBestPathResult(response.data);
    } catch (error) {
      console.error("Error finding best path:", error.message);
    }
  };

  const handleNumSalespeopleChange = (e) => {
    const value = Math.max(1, parseInt(e.target.value) || 1); // Ensure a minimum of 1
    setNumSalespeople(value);
  };

  return (
    <>
      <Header/>
      <div className="app-container">
        <br></br>

        <div className="input-section">
          {/* Salespeople Input */}
          <div className="salespeople-input">
            <label htmlFor="num-salespeople" className="label">
              Number of Salespeople:
            </label>
            <input
              type="number"
              id="num-salespeople"
              value={numSalespeople}
              onChange={handleNumSalespeopleChange}
              className="input-box"
              min="1"
            />
          </div>

          {/* Locations Input */}
          <h4>Enter the city names</h4>
          {locations.map((location, index) => (
            <input
              key={index}
              value={location}
              onChange={(e) => handleLocationChange(index, e.target.value)}
              placeholder={`Enter Location ${index + 1}`}
              className="input-box"
            />
          ))}
          <button onClick={handleAddLocation} className="button add-button">
            Add Location
          </button>
        </div>

        <button
          onClick={handleCalculateMatrix}
          className="button calculate-button"
        >
          Calculate Distance Matrix
        </button>

        {distanceMatrix.length > 0 && (
          <div className="matrix-section">
            <h2>Distance Matrix</h2>
            <table className="matrix-table">
              <thead>
                <tr>
                  <th></th>
                  {locations.map((loc, index) => (
                    <th key={index}>{loc}</th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {distanceMatrix.map((row, rowIndex) => (
                  <tr key={rowIndex}>
                    <td>{locations[rowIndex]}</td>
                    {row.map((distance, colIndex) => (
                      <td key={colIndex}>
                        {distance != null ? distance.toFixed(2) + " km" : "N/A"}
                      </td>
                    ))}
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}

        <button
          onClick={handleFindBestPath}
          className="button find-button"
          disabled={distanceMatrix.length === 0}
        > Find Best Path </button>

        {bestPathResult && (
          <div className="result-section">
            <h2>Best Path</h2>
            <p>Total Distance: {bestPathResult.totalDistance} km</p>

            <h3>Individual Distances</h3>
            <ul>
              {bestPathResult.individualDistances.map((distance, index) => (
                <li key={index}>Salesperson {index}: {distance} km</li>
              ))}
            </ul>

            <h3>Salesperson Visits</h3>
            <ul>
              {bestPathResult.salespersonVisits.map((salesperson, index) => (
                <li key={index}>
                  Salesperson {salesperson.salesperson} visits:{" "}
                  {Array.isArray(salesperson.visits)
                    ? salesperson.visits.join(", ")
                    : "No visits"}
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>
    </>
  );
};

export default App;
