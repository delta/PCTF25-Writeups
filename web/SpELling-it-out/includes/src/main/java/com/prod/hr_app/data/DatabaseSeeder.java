package com.prod.hr_app.data;

import com.prod.hr_app.model.User;
import com.prod.hr_app.repository.UserRepository;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;

@Component
public class DatabaseSeeder implements CommandLineRunner {

    private final UserRepository userRepository;

    public DatabaseSeeder(UserRepository userRepository) {
        this.userRepository = userRepository;
    }

    @Override
    public void run(String... args) throws Exception {
        userRepository.save(new User("1", "john_doe", "developer"));
        userRepository.save(new User("2", "jane_smith", "manager"));
        userRepository.save(new User("3", "bruce_wayne", "developer"));
        userRepository.save(new User("4", "clark_kent", "Senior manager"));
        userRepository.save(new User("5", "diana_prince", "tester"));
        userRepository.save(new User("6", "tony_stark", "President"));
        userRepository.save(new User("7", "steve_rogers", "manager"));
        userRepository.save(new User("8", "natasha_romanoff", "developer"));
        userRepository.save(new User("9", "peter_parker", "designer"));
        userRepository.save(new User("10", "logan_howlett", "tester"));
        userRepository.save(new User("11", "charles_xavier", "developer"));
        userRepository.save(new User("12", "erik_lehnsherr", "manager"));
        userRepository.save(new User("13", "jean_grey", "developer"));
        userRepository.save(new User("14", "scott_summers", "tester"));
        userRepository.save(new User("15", "ororo_munroe", "manager"));
        userRepository.save(new User("16", "wanda_maximoff", "developer"));
        userRepository.save(new User("17", "stephen_strange", "designer"));
        userRepository.save(new User("18", "shuri", "developer"));
        userRepository.save(new User("19", "tchalla", "manager"));
        userRepository.save(new User("20", "bucky_barnes", "tester"));
        userRepository.save(new User("21", "sam_wilson", "developer"));
        userRepository.save(new User("22", "pepper_potts", "designer"));
        userRepository.save(new User("23", "happy_hogan", "developer"));
        userRepository.save(new User("24", "nick_fury", "Vice President"));
        userRepository.save(new User("25", "carol_danvers", "tester"));
        userRepository.save(new User("26", "peter_quill", "developer"));
        userRepository.save(new User("27", "gamora", "designer"));
        userRepository.save(new User("28", "drax_destroyer", "manager"));
        userRepository.save(new User("29", "rocket_raccoon", "tester"));
        userRepository.save(new User("30", "groot", "developer"));
        userRepository.save(new User("31", "thor_odinson", "developer"));
        userRepository.save(new User("32", "loki_laufeyson", "manager"));
        userRepository.save(new User("33", "valkyrie", "developer"));
        userRepository.save(new User("34", "hela", "designer"));
        userRepository.save(new User("35", "odin", "manager"));
        userRepository.save(new User("36", "frigga", "tester"));
        userRepository.save(new User("37", "korg", "developer"));
        userRepository.save(new User("38", "meik", "designer"));
        userRepository.save(new User("39", "hulk", "manager"));
        userRepository.save(new User("40", "bruce_banner", "tester"));
        userRepository.save(new User("41", "jessica_jones", "developer"));
        userRepository.save(new User("42", "luke_cage", "manager"));
        userRepository.save(new User("43", "danny_rand", "tester"));
        userRepository.save(new User("44", "matt_murdock", "developer"));
        userRepository.save(new User("45", "elektra_natchios", "designer"));
        userRepository.save(new User("46", "frank_castle", "manager"));
        userRepository.save(new User("47", "karan_gill", "tester"));
        userRepository.save(new User("48", "deep_singh", "developer"));
        userRepository.save(new User("49", "anjali_das", "designer"));
        userRepository.save(new User("50", "arjun_nair", "manager"));
        userRepository.save(new User("51", "zoya_shah", "tester"));
        userRepository.save(new User("52", "ahmed_khan", "developer"));
        userRepository.save(new User("53", "sara_rathod", "manager"));
        userRepository.save(new User("54", "ryan_cooper", "developer"));
        userRepository.save(new User("55", "emily_davis", "tester"));
        userRepository.save(new User("56", "josh_evans", "designer"));
        userRepository.save(new User("57", "rachel_moore", "developer"));
        userRepository.save(new User("58", "oliver_stone", "manager"));
        userRepository.save(new User("59", "harper_lee", "tester"));
        userRepository.save(new User("60", "miles_morales", "developer"));
        userRepository.save(new User("61", "gwen_stacy", "designer"));
        userRepository.save(new User("62", "eddie_brock", "manager"));
        userRepository.save(new User("63", "cletus_kasady", "tester"));
        userRepository.save(new User("64", "norman_osborn", "developer"));
        userRepository.save(new User("65", "harry_osborn", "manager"));
        userRepository.save(new User("66", "mary_jane", "tester"));
        userRepository.save(new User("67", "felicia_hardy", "developer"));
        userRepository.save(new User("68", "max_dillon", "designer"));
        userRepository.save(new User("69", "adrian_toomes", "manager"));
        userRepository.save(new User("70", "otto_octavius", "tester"));
        userRepository.save(new User("71", "gwen_tennyson", "developer"));
        userRepository.save(new User("72", "kevin_levin", "manager"));
        userRepository.save(new User("73", "ben_tennyson", "developer"));
        userRepository.save(new User("74", "julie_yamamoto", "designer"));
        userRepository.save(new User("75", "grandpa_max", "manager"));
        userRepository.save(new User("76", "peter_gabriel", "developer"));
        userRepository.save(new User("77", "simone_willis", "manager"));
        userRepository.save(new User("78", "barbara_white", "designer"));
        userRepository.save(new User("79", "victor_hudson", "developer"));
        userRepository.save(new User("80", "amelia_johnson", "tester"));
        userRepository.save(new User("81", "nina_hart", "developer"));
        userRepository.save(new User("82", "carlos_rivera", "manager"));
        userRepository.save(new User("83", "karen_jones", "developer"));
        userRepository.save(new User("84", "lucas_black", "designer"));
        userRepository.save(new User("85", "bella_gray", "developer"));
        userRepository.save(new User("86", "hannah_martin", "manager"));
        userRepository.save(new User("87", "leo_turner", "tester"));
        userRepository.save(new User("88", "aria_scott", "developer"));
        userRepository.save(new User("89", "ian_clark", "designer"));
        userRepository.save(new User("90", "freya_wood", "manager"));
        userRepository.save(new User("91", "emma_reed", "tester"));
        userRepository.save(new User("92", "aiden_foster", "developer"));
        userRepository.save(new User("93", "riley_adams", "manager"));
        userRepository.save(new User("94", "liam_morris", "tester"));
        userRepository.save(new User("95", "zoe_watson", "developer"));
        userRepository.save(new User("96", "isla_bailey", "designer"));
        userRepository.save(new User("97", "jack_evans", "manager"));
        userRepository.save(new User("98", "amelia_brown", "tester"));
        userRepository.save(new User("99", "olivia_white", "developer"));
        userRepository.save(new User("100", "sophia_moore", "manager"));
    }
}
