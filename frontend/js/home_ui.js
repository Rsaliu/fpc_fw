let isLoggedIn = localStorage.getItem("isLoggedIn") === "true";

function redirectTo(page) {
  if (page === 'config_ui.html' && !isLoggedIn) {
    alert("Please log in to access configuration.");
    return;
  }
  window.location.href = page;
}

function updateUI() {
  const loginTabLink = document.getElementById("loginTabLink");
  const registerTabLink = document.getElementById("registerTabLink");
  const configTabLink = document.getElementById("configTabLink");
  const resetTabLink = document.getElementById("resetTabLink");

  if (isLoggedIn) {
    loginTabLink.style.display = "none";
    registerTabLink.style.display = "none";
    configTabLink.style.display = "inline-block";
    resetTabLink.style.display = "inline-block";
    configTabLink.disabled = false;
    resetTabLink.disabled = false;
  } else {
    loginTabLink.style.display = "inline-block";
    registerTabLink.style.display = "inline-block";
    configTabLink.style.display = "none";
    resetTabLink.style.display = "none";
    loginTabLink.textContent = "Login";
    loginTabLink.onclick = () => redirectTo('login_ui.html');
  }
}

function reset() {
  isLoggedIn = false;
  localStorage.removeItem("isLoggedIn");
  updateUI();
  alert("Login details cleared successfully!");
  window.location.href = 'login_ui.html';
}

// Initialize the UI
document.addEventListener("DOMContentLoaded", updateUI);