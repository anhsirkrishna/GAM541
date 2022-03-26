--Constants
left_action = 0
right_action = 1
jump_action = 2
attack_action = 3
burst_jump_action = 4
time_till_health_disable = 120

pressed = 0
triggered = 1
released = 2
inactive = 3

walk_speed = 100;
jump_speed = -120;

die_event_id = 5

timer = timer + adder

function Die()
	if get_state() ~= "DIE" then
		change_state("DIE")
		set_transform_scale(1 * hit_direction, 1)  --Make sure the sprite is facing right
		--Send an event id 5, with a delay of 0 ms, with broadcast=false
		send_event(5, 0, false)
	end
end

function MoveCharacterRight()
	--Only move if character is already walking or idle
	--if get_state() == "IDLE" or get_state() == "WALK" then
		move(walk_speed) --Move with a velocity of "walk_speed"
		set_transform_scale(1, 1)  --Make sure the sprite is facing right
		if get_state() == "WALK" or get_state() == "IDLE" then
			change_state("WALK")
		end
	--end
end

function MoveCharacterLeft()
	--Only move if character is already walking or idle
	--if get_state() == "IDLE" or get_state() == "WALK" then
		move(-1*walk_speed) --Move with a velocity of "walk_speed"
		set_transform_scale(-1, 1)  --Make sure the sprite is facing right
		if get_state() == "WALK" or get_state() == "IDLE" then
			change_state("WALK")
		end
	--end
end

function JumpCharacter()
	--Cannot double jump so make sure character not jumping/falling
	if get_state() ~= "JUMP" and get_state() ~= "FALL" then
		jump(jump_speed)
		change_state("JUMP")
	end
end

function BurstJump()
	if get_state() == "IDLE" then
		jump(jump_speed*1.2)
		move(walk_speed * get_scale_x())
		change_state("BURST_JUMP")
	end
end

function ThrowProjectile()
	if get_state() == "IDLE" then
		spawn_projectile()
	end
end

-- You can only do stuff - if you're not dead
if get_state() ~= "DIE" then

	-- Let the attack animation finish before doing anything else
	if get_state() == "ATTACK" then
		if is_animation_completed() == true then
			change_state("IDLE")
		else
			return
		end
	end

	if check_action_state(left_action, inactive) and 
		check_action_state(right_action, inactive) and 
		check_action_state(jump_action, inactive) then
			change_state("IDLE")
			move(0)
	end

	--If right was triggered
	if check_action_state(right_action, pressed) then
		MoveCharacterRight()
	end

	--If left was triggered
	if check_action_state(left_action, pressed) then
		MoveCharacterLeft()
	end

	--If right was released while left was pressed
	if check_action_state(right_action, released) and check_action_state(left_action, pressed) then
		MoveCharacterLeft()
	end

	--If left was released while right was pressed
	if check_action_state(right_action, released) and check_action_state(left_action, pressed) then
		MoveCharacterRight()
	end

	--If jump button was triggered
	if check_action_state(jump_action, triggered) then
		JumpCharacter()
	end

	if check_action_state(burst_jump_action, triggered) then
		BurstJump()
	end


	--If character is falling
	if get_vertical_velocity() > 0 then
		change_state("FALL")
	elseif get_vertical_velocity() == 0 then
		if get_state() == "FALL" then
			change_state("IDLE")
		end
	end

	if check_action_state(attack_action, triggered) then
	--	ThrowProjectile()
		change_state("ATTACK")
	end

	-- timer till health can be decremented again
	if timer > 0 then
		timer = timer - 5
	end


	if received_event then
		if hit_event and timer <= 0 then
			decr_health()	
			timer = time_till_health_disable
			if get_health() == 0 then
				Die()
			end
		end
	end

end
