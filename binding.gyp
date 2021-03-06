{
	"targets": [{
		"target_name": "desktop",
		"defines": [ "DEBUG" ],

		"include_dirs": ["<(module_root_dir)/includes","yoga"],
		"sources": [
			'<!@(node tools/list-sources.js)'
		],
		"cflags_cc": ['-fexceptions'],
		
		"conditions": [
			["OS=='win'", {
			}],
			["OS=='mac'", {
				"libraries": ["-framework AppKit"]
			}],
			["OS=='linux'", {
				"libraries": [
					'<!@(pkg-config gtk+-3.0 --libs-only-l)',
					'<!@(pkg-config gdk-pixbuf-2.0 --libs-only-l)'
				],
				
				"cflags": [
					'<!@(pkg-config gtk+-3.0 --cflags-only-I)',
					'<!@(pkg-config gdk-pixbuf-2.0 --cflags-only-I)'
				]
			}]

		]
	}]
}
