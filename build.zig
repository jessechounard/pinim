const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "pinim",
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(b.path("dependencies"));
    exe.addIncludePath(b.path("include"));
    exe.addCSourceFiles(.{
        .files = &.{
            "dependencies/glad/gl.c",
            "source/graphics/BatchRenderer.c",
            "source/graphics/GraphicsDevice.c",
            "source/graphics/ShaderProgram.c",
            "source/graphics/Texture.c",
            "source/graphics/VertexBuffer.c",
            "source/main.c",
        },
        .flags = &.{
            "-Wall",
            "-Werror",
        },
    });

    const sdl_dep = b.dependency("sdl", .{
        .target = target,
        .optimize = optimize,
        .preferred_link_mode = .static,
    });
    const sdl_lib = sdl_dep.artifact("SDL3");

    exe.root_module.linkLibrary(sdl_lib);

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the game");
    run_step.dependOn(&run_cmd.step);
}
