const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "pinim",
        .target = target,
        .optimize = optimize,
    });
    exe.addIncludePath(b.path("src/dependencies"));
    exe.addCSourceFiles(.{
        .files = &.{
            "src/dependencies/glad/gl.c",
            "src/GraphicsDevice.c",
            "src/main.c",
            "src/ShaderProgram.c",
            "src/VertexBuffer.c",
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
