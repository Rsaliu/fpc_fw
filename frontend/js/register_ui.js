function register(event) {
  event.preventDefault(); // Prevent form submission (if inside a form)

  const form = document.getElementById("registerForm");

  // Collect form values
  const formData = {
    username: form.username.value.trim(),
    password1: form.pwd1.value,
    password2: form.pwd2.value
  };

  // Optional: Check if passwords match
  if (formData.pwd1 !== formData.pwd2) {
    alert("Passwords do not match.");
    console.log("Passwords do not match.");
    return;
  }
  console.log("will send data no,password match");
  // Send the data as JSON (e.g., to an API endpoint)
  fetch("https://fpc-webserver.local/register", {
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
    alert("Registration successful!");
    console.log("Server response:", data);
  })
  .catch(error => {
    console.error("Error:", error);
    alert("Registration failed!");
  });
}
