import React from "react";
import "./App.css"; // Import global styles

const Header = () => {
  return (
    <header className="header">
      <div className="header-container">
        <h1 className="header-title">Traveling Salesman Problem</h1>
        <nav className="header-nav">
          <button className="nav-button">Upload File</button>
          <button className="nav-button">About</button>
          <button className="nav-button">Contact</button>
        </nav>
      </div>
    </header>
  );
};

export default Header;
