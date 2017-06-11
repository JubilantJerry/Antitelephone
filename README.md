# Antitelephone

## Overview

This project is a new concept for a multiplayer time travel game inspired by the parallel universe theory. I wanted to make a game like this because so few multiplayer games use time travel as a central game element (instead of, say, a plot element).

The only true multiplayer time travel game I could find online was <a href="http://www.achrongame.com/site/">Achron</a>, an RTS game that allows players to alter past events. That said there are elements that transcend in-game time, giving some sense of "meta-time" instead. This includes things like chrono-energy, the immutable past and time-waves, all of which the player cannot control. With this, many time-related concerns are still present. Players can still regret past decisions and have a limited amount of time to react to events in the game.

Unlike Achron, I didn't want to push the responsibility of time toward some kind of "meta-time". Instead, players can use a time machine (a magical item that behaves like a <a href="https://en.wikipedia.org/wiki/Tachyonic_antitelephone">tachyonic antitelephone</a>) which has the potential to send messages to any point in their own past. This naturally brings many problems in game design, and I tried my best to make a meaningful game without compromising the time travel element.

## Cursory Explanation of Game Mechanics

The game is turn-based, since it's more or less impossible to warp time in a real-time game. There are a small number of rooms in a game, and the player can visit any one of those rooms each turn. The moves of all players are processed simultaneously, unlike other turn-based games where the first player or the player with the highest speed stat makes the first move. If multiple players meet in the same room, combat occurs instantly and automatically.

Without the time travel element, the game is strongly dependant on luck. Players in general don't know where other players are, except when they meet at the same room. All players start on the same footing and there's only a few options to increase damage output, though there is a way to heal in between encounters. I suspect that it wouldn't be hard to come up with a strategy to maximize the probability of winning for this no-time-travel variant of this game.

In this game, time travel doesn't feel like something useless you can do to delay the inevitable. It's actually a necessary element to use future knowledge to your advantage. Using the Antitelephone to go back in time creates a new timeline where you can make independent decisions while your opponents repeat their past actions (as long as you don't reveal your presence). For good measure, the Antitelephone also tells you the location of the opponent in the previous timeline so you can plan accordingly. You can launch surprise attacks when your enemies are at their weakest, or greedily build up your health and skillset without worring about being attacked. There are certain sacrifices you have to make to travel in time though, so you have to use the Antitelephone strategically.

Other time-related items exist in the game too, which can be unlocked with persistent effort. In addition to increasing damage output and maximum health, they also offer abilities that complement the Antitelephone. They could make more points in time accessible to time travel, notify past versions of you about moving to different timelines, or act as a shield that can absorb damage and be carried to the past. If you're interested, play the game when it comes out! Or read the source code to see what's going on, if you're into that kind of stuff.

## Details

This game is written in C++, and if all goes well I'll try to support network multiplayer games of 2 - 6 players. I'm not going to buy a dedicated server to do this, so players would have to set up the server on their own machine or something.

Part of the reason I'm making this game is to learn how to program effectively in C++. To make sure I don't get sloppy, I designed a detailed object model to guide my progress and will use Catch to make unit tests. If you want to help, you can send me an email (listed in my public profile). Of course, the game is open-source and you can make a fork of my branch if you want.
