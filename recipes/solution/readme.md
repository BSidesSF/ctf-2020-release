Too lazy to write a fully automated test, but:

- Add a recipe, use a SSRF to localhost /users to get a list of users.
- Use a JWT with "none" method and the UUID for the boudin user.
