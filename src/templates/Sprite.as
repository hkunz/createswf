package
{
	import flash.display.Bitmap;

	public class ${name} extends ExtendedSprite {
		${loop1}[Embed(source="${path}", mimeType="${mime}")] private var bmpref${count}:Class;

		public function ${name} () {
			super ();
			${loop2}addObject(new bmpref${count}() as Bitmap, ${x}, ${y}, ${alpha}, ${visible});
			this.x = ${x};
			this.y = ${y};
			this.alpha = ${alpha};
			this.visible = ${visible};
		}
	}
}