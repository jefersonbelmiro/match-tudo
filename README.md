# match-tudo: puzzle game 

```
basead on:
    mecanic:
     - https://www.reddit.com/r/godot/comments/1tuul22/built_a_chill_countryhopping_puzzle_game_in_godot/
     - https://dailyjig.com/
    colors and animations:
     - https://www.youtube.com/watch?v=F1I0lzM_UZI&t=63s
```


idea is to solve memory puzzle, shuffled
vibe retro, satured colors
modes: 
    - cozy, light sound, no timer
    - fast-pace, with energetic music and timer
scoreboard global, may by timer, by points, some score to be defined


# perf

```bash
perf stat -d -d -d build/match-tudo
perf stat -e cache-misses,cache-references,L1-dcache-load-misses,L1-dcache-loads build/match-tudo
```
