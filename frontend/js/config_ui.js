function loadConfig() {
  if (localStorage.getItem("isLoggedIn") !== "true") {
    alert("Please log in to access configuration.");
    window.location.href = "login_ui.html";
    return;
  }

  fetch("https://fpc-webserver.local/config/load", {
    method: "GET",
    headers: {
      "Content-Type": "application/json"
    
    }
  })
  .then(response => {
    if (!response.ok) {
      throw new Error(`Failed to load configuration: ${response.statusText}`);
    }
    return response.json();
  })
  .then(data => {
    document.getElementById("configInput").value = JSON.stringify(data, null, 2);
    document.getElementById("status").textContent = "Configuration loaded successfully!";
  })
  .catch(error => {
    console.error("Error:", error);
    document.getElementById("status").textContent = `Error loading configuration: ${error.message}`;
  });
}

function saveConfig() {
  if (localStorage.getItem("isLoggedIn") !== "true") {
    alert("Please log in to access configuration.");
    window.location.href = "login_ui.html";
    return;
  }

  const configInput = document.getElementById("configInput").value;
  let configData;
  try {
    configData = JSON.parse(configInput);
  } catch (error) {
    document.getElementById("status").textContent = "Invalid JSON format. Please check your input.";
    return;
  }

  fetch("https://fpc-webserver.local/config/save", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
      // Add authentication header if required, e.g., "Authorization": "Bearer " + localStorage.getItem("token")
    },
    body: JSON.stringify(configData)
  })
  .then(response => {
    if (!response.ok) {
      throw new Error(`Failed to save configuration: ${response.statusText}`);
    }
    return response.json();
  })
  .then(data => {
    document.getElementById("status").textContent = "Configuration saved successfully!";
  })
  .catch(error => {
    console.error("Error:", error);
    document.getElementById("status").textContent = `Error saving configuration: ${error.message}`;
  });
}