yapt
====

Component based realtime rendering engine.
Compose scenes and put them on a timeline using XML.

Supplied Plugins:
- Rendering for "old" OpenGL.
- Geometry generators
- Interpolation
- Music player (mp3)
- Various utility stuff

Runs on Mac OS and should be portable to most platforms (known to work on Windows).
Depends on GLFW for core OpenGL.

The Engine itself has no dependency on OpenGL can be replaced with DirectX/Vulcan or Metal.

**Example** (renders a point cloud):

```xml
<yapt2>
	<resources>
		<object class="geom.PointCloud" name="data.starfield">
			<property name="numVertex">2048</property>
			<property name="range">1.0, 0.5, 1.0</property>
		</object>
	</resources>

	<render>
		<object class="gl.RenderContext" name="mycontext">
			<property name="fov">35.0</property>

			<object class="gl.DrawPoints" name="render_drawstars">
				<property name="vertexCount" source="data.starfield.vertexCount" />
				<property name="vertexData" source="data.starfield.vertexData" />
			</object>
		</object>
	</render>
	<timeline>
		<execute start="0" duration="1000" object="mycontext" />
	</timeline>
</yapt2>
```
