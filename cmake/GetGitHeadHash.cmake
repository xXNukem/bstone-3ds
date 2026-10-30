#[[
Read the source revision when Git metadata and the optional Git client are available.

Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.
SPDX-License-Identifier: MIT

Source archives have no revision suffix. BSTONE_CI_GIT_HASH can supply one
without a Git client. Git handles normal checkouts, packed refs and worktrees.
]]

function(get_git_hash DIR HASH_VAR)
    set(${HASH_VAR} "" PARENT_SCOPE)
    if (NOT EXISTS "${DIR}/.git")
        return()
    endif ()

    find_package(Git QUIET)
    if (NOT GIT_FOUND)
        return()
    endif ()

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${DIR}" rev-parse --verify HEAD
        RESULT_VARIABLE git_result
        OUTPUT_VARIABLE git_hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (git_result EQUAL 0)
        set(${HASH_VAR} "${git_hash}" PARENT_SCOPE)
    endif ()
endfunction()
