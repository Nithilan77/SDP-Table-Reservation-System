// This function is for logging out the user
function logout() {
    localStorage.removeItem("userRole");
    window.location.href = "index.html";
}

// This function sets up the navigation dynamically based on user role
function setupNavigation() {
    let userRole = localStorage.getItem("userRole");
    let nav = document.getElementById("dynamicNav");

    if (!userRole) {
        window.location.href = "index.html";
        return;
    }

    let navItems = `
        <li class="nav-item"><a class="nav-link" href="home.html">Home</a></li>
        <li class="nav-item"><a class="nav-link" href="menu.html">Menu</a></li>
    `;

    if (userRole === "user") {
        navItems += `<li class="nav-item"><a class="nav-link" href="contact.html">Contact</a></li>`;
        navItems += `<li class="nav-item"><a class="nav-link" href="reservation.html">Reserve Table</a></li>`;
    } else {
        navItems += `<li class="nav-item"><a class="nav-link" href="admin_dashboard.html">Reserved Tables</a></li>`;
    }

    navItems += `<li class="nav-item"><a class="nav-link text-danger" href="#" onclick="logout()">Logout</a></li>`;
    nav.innerHTML = navItems;
}

// Run on page load
window.onload = () => {
    const params = new URLSearchParams(window.location.search);
    const role = params.get("role");
    if (role) localStorage.setItem("userRole", role);
    setupNavigation();
};

