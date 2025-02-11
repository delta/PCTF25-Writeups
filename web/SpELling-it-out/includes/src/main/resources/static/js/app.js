const searchForm = document.getElementById('search-form');
const resultsDiv = document.getElementById('results');
const viewButtons = document.querySelectorAll('.view-btn');

const API_ENDPOINTS = {
    SEARCH: '/api/users/search'
};

searchForm.addEventListener('submit', handleSearch);
viewButtons.forEach(btn => btn.addEventListener('click', handleViewChange));

async function handleSearch(event) {
    event.preventDefault();
    const username = document.getElementById('username').value;

    const encodedUsername = btoa(unescape(encodeURIComponent("'" + username + "'")))
        .replace(/\+/g, '-')
        .replace(/\//g, '_')
        .replace(/=+$/, '');

    showLoadingState();

    try {
        const result = await searchUsers(encodedUsername);
        displayResults(result);
    } catch (error) {
        showError(error);
    }
}

async function searchUsers(encodedUsername) {
    try {
        const response = await fetch(`${API_ENDPOINTS.SEARCH}?username=${encodedUsername}`, {
            method: 'GET',
        });

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }

        return await response.json();
    } catch (error) {
        console.error('Search request failed:', error);
        throw new Error('Failed to search for users. Please try again.');
    }
}


function showLoadingState() {
    resultsDiv.innerHTML = `
        <div class="loading">
            <i class="fas fa-circle-notch fa-spin"></i>
            <p>Searching...</p>
        </div>
    `;
}

function displayResults(users) {
    if (!users || users.length === 0) {
        showNoResults();
        return;
    }

    const userCards = users.map(user => `
        <div class="user-card">
            <h3>${escapeHtml(user.username)}</h3>
            <p>Role: ${escapeHtml(user.role)}</p>
        </div>
    `).join('');

    resultsDiv.innerHTML = `
        <div class="user-results">
            ${userCards}
        </div>
    `;
}


function showNoResults() {
    resultsDiv.innerHTML = `
        <div class="no-results">
            <i class="fas fa-search"></i>
            <p>No users found matching your search.</p>
        </div>
    `;
}

function showError(error) {
    resultsDiv.innerHTML = `
        <div class="error">
            <i class="fas fa-exclamation-circle"></i>
            <p>${error.message}</p>
        </div>
    `;
}

function handleViewChange(event) {
    const selectedView = event.currentTarget.dataset.view;
    
    viewButtons.forEach(btn => {
        btn.classList.toggle('active', btn.dataset.view === selectedView);
    });

    resultsDiv.className = `results-${selectedView}`;
}

function escapeHtml(unsafe) {
    return unsafe
        .toString()
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

document.addEventListener('DOMContentLoaded', () => {
    document.querySelector('[data-view="grid"]').classList.add('active');
    resultsDiv.className = 'results-grid';
});