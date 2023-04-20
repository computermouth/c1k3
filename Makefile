
EXT_SRC = lodepng.c
MAIN_C  = main.c
INT_SRC = data.c entity_barrel.c entity.c entity_door.c entity_enemy.c entity_enemy_enforcer.c entity_enemy_grunt.c entity_enemy_ogre.c entity_enemy_zombie.c entity_enemy_hound.c entity_light.c entity_particle.c entity_pickup.c entity_pickup_grenadelauncher.c entity_pickup_grenades.c entity_pickup_health.c entity_pickup_key.c entity_pickup_nailgun.c entity_pickup_nails.c entity_player.c entity_projectile_gib.c entity_projectile_grenade.c entity_projectile_nail.c entity_projectile_plasma.c entity_projectile_shell.c entity_torch.c entity_trigger_level.c game.c input.c map.c math.c model.c render.c weapon.c
INT_H   = data.h entity_barrel.h entity.h entity_door.h entity_enemy.h entity_enemy_enforcer.h entity_enemy_grunt.h entity_enemy_ogre.h entity_enemy_zombie.h entity_enemy_hound.h entity_light.h entity_particle.h entity_pickup.h entity_pickup_grenadelauncher.h entity_pickup_grenades.h entity_pickup_health.h entity_pickup_key.h entity_pickup_nailgun.h entity_pickup_nails.h entity_player.h entity_projectile_gib.h entity_projectile_grenade.h entity_projectile_nail.h entity_projectile_plasma.h entity_projectile_shell.h entity_torch.h entity_trigger_level.h game.h input.h map.h math.h model.h render.h weapon.h
TST_SRC = tests/test.c
SAN_FLAGS = -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
SAN_OPT = ASAN_OPTIONS=abort_on_error=1:fast_unwind_on_malloc=0:detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1

all:
	clang -Wall -g $(SAN_FLAGS) $(MAIN_C) $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2 -lSDL2

release:
	clang -Os -flto -Wall $(MAIN_C) $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2 -lSDL2
	strip main

valbuild:
	clang -Wall -g $(MAIN_C) $(INT_SRC) $(EXT_SRC) -o main -lm -lGLESv2 -lSDL2

memtest: valbuild
	valgrind --track-origins=yes --leak-check=yes --gen-suppressions=all --suppressions=extra/suppressions.valg ./main

lint:
	astyle -n $(MAIN_C) $(INT_SRC) $(INT_H) $(TST_SRC)

test:
	## super overkill with the linking and building in all objects
	clang -Wall $(INT_SRC) $(EXT_SRC) tests/*.c -o tester -lSDL2 -lm -lGLESv2
	./tester

debug: all
	$(SAN_OPT) lldb ./main
