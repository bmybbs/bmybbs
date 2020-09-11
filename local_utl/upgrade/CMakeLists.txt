add_executable(upgrade upgrade.c)
target_link_libraries(upgrade ythtbbs ytht)

add_executable(upgrade.passwds.to.ipv6 upgrade.passwds.to.ipv6.c)
target_link_libraries(upgrade.passwds.to.ipv6 ythtbbs ytht)