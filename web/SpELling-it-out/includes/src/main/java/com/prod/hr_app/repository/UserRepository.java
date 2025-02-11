package com.prod.hr_app.repository;

import com.prod.hr_app.model.User;
import org.springframework.data.mongodb.repository.Query;
import org.springframework.data.repository.CrudRepository;
import org.springframework.stereotype.Repository;
import org.springframework.data.repository.query.Param;

import java.util.List;

@Repository
public interface UserRepository extends CrudRepository<User, String> {
    @Query("{ 'username': { $regex: ?#{?0} } }")
    List<User> findByUsernameContaining(String username);
}