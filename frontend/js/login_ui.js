function login(event) {
  event.preventDefault(); // Prevent form submission (if inside a form)

  const form = document.getElementById("loginForm");

  // Collect form values
  const formData = {
    username: form.username.value.trim(),
    password: form.pwd.value
  };

  
  // Send the data as JSON (e.g., to an API endpoint)
  fetch("https://fpc-webserver.local/login", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(formData)
  })
  .then(response => {
    if (!response.ok) {
      throw new Error("Network response was not OK");
    }
    return response.json();
  })
  .then(data => {
    alert("Login successful!");
    console.log("Server response:", data);
  })
  .catch(error => {
    console.error("Error:", error);
    alert("Invalid Cridentials!");
  });
}


