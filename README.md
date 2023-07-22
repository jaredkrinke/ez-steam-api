# ez-steam-api
This is a synchronous/blocking, flat, C-style wrapper around the [Steamworks SDK](https://partner.steamgames.com/doc/sdk), originally designed for use via foreign function interfaces.

## Motivation
The Steamworks API is native C++, with an optional flat/C-style API. Unfortunately, I found both provided options cumbersome in my project, for a few reasons:

* Callbacks are designed to be polled each frame--a hassle for event-driven environments
* Callbacks and call results are C++ member functions, with no out-of-the-box support for blocking (synchronous results) or promises
* The language I was using (JavaScript) doesn't have a good C++ interop story (whereas C interop is ubiquitous)

## Design
Internally, ez-steam-api spins up a thread for running Steam's callbacks and takes care of marshaling results back to calling threads (who just see a trivial synchronous API).

The synchronous, flat API can be trivially used with C-based foreign function interfaces that many languages provide. For example, this repository provides a Node/JavaScript wrapper, based on [Koffi](https://koffi.dev/).

## Versioning and compatibility
The second (minor) version number corresponds to the Steamworks SDK's second version component. For example, X.**57**.Y is major version X, targeting Steamworks SDK 1.**57**, with patch version Y.

## Status
Note that this library currently only exposes the portions of the Steamworks API that I needed for my own game. The code quality is "hobby ready" (as opposed to "production ready").