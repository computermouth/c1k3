msgpack format

objects are maps
`/*this is a comment*/`
TEXTURE_BYTES is not a string, but a msgpack byte type

```
{
	"map_cubes": [
		{"size": [1,1,1], "start": [1,1,1], "tex_id": 0},
		{"size": [2,2,2], "start": [2,2,2], "tex_id": 1}
	],
	"map_entts": [
		{"pos": [1.0,1.0,1.0], "tex_id": 0},
		{"pos": [2.0,2.0,2.0], "tex_id": 1}
	],
	"map_lites": [
		{"color": [255,255,255,255], "pos": [1.0,1.0,1.0]},
		{"color": [255,255,255,255], "pos": [2.0,2.0,2.0]}
	],
	"map_player": [1.0,2.0,3.0],
	"ref_cubes": [ "TEXTURE_BYTES", "TEXTURE_BYTES", "TEXTURE_BYTES" ],
	"ref_entts": [
		{
			"/*object is entt*/":"",
			"anim_frames": [
				[ ["/*default*/"],
					[1.0,1.0,1.0],
					[1.0,1.0,1.0]
				],
				[ ["/*Key 1*/"],
					[1.0,1.0,1.0],
					[1.0,1.0,1.0]
				]
			],
			"anim_names": [ "default", "Key 1" ],
			"txtr": "TEXTURE_BYTES",
			"u": [1.0, 1.0],
			"v": [1.0, 1.0]
		}
	]
}
```
