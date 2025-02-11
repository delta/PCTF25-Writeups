package com.prod.hr_app.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import com.prod.hr_app.model.User;
import com.prod.hr_app.repository.UserRepository;
import java.util.Base64;
import java.util.List;

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
            return ResponseEntity.status(500).body("An unexpected error occurred.");
        }
    }
}
