projectile_speed = 2;

--Get the current xpos
x_coord = get_pos_coord(0)

--Get the xpos after moving it
new_x_coord = x_coord + (projectile_speed * get_scale_x())
set_pos_coord(new_x_coord, 0)

if received_event then
	if impact_event then
		particle_burst(100)
		--disable after half a second
		delayed_disable_obj(500)
	end
end