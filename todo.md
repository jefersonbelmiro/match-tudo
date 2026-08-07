# TODO

## board
```

storage_t {
  // float    gold;
  map_state_t *map_states;
  u16          map_states_count;
}

map_t {
  id: number,
  name: string
  category: string
  levels: []
}

map_table_t {
  map_t array[10];
  u16   count;
}

map_state_t {
  map_id: int,
  level_index: int,
  level_completed_count: int
  level_completed: int[]
}

map_pack_t {
  map_t           *map;
  resource_pack_t *resources;
}

resource_pack_t {
  textures: [],
  sounds: [],
  musics: [],
}


+--------------+
|[ hud/topbar ]|
| -- pad: 5 -- |
|   [][][][]   |
|   [][][][]   |
|   [][][][]   |
| -- pad: 5 -- |
|[ hud/footer ]|
+--------------+

```




# backlog
 - [ ] - editor generate pack level and store/load
 - [ ] - editor can turn image to pixel art like pixelit(js lib)
 - [ ] - remove signal from nodes, use flag pressed,changed,...etc
 - [ ] - powerups
        >> solver: throw in cell range and solve that area(animated)

editor:
 - [ ] - create levels
 - [ ] - configure board level
        >> configure image offset and saturation based on board size
