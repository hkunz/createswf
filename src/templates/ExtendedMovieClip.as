package
{
	import flash.display.Bitmap;
	import flash.display.DisplayObject;
	import flash.display.MovieClip;

	public class ExtendedMovieClip extends MovieClip {
		private var _frames:${arraytype} = new ${arraytype}();
		private var _frame:int = 1;

		public function ExtendedMovieClip () {
			super ();
		}

		public override function gotoAndPlay (frame:Object, scene:String = null):void {
			showframe(int(frame))
		}

		public override function gotoAndStop (frame:Object, scene:String = null):void {
			showframe(int(frame))
		}

		public override function nextFrame ():void {
			showframe(int(_frame + 1))
		}

		public override function prevFrame ():void {
			showframe(int(_frame - 1))
		}

		protected function addObject (object:DisplayObject, x:int = 0, y:int = 0, alpha:Number = 1.0, visible:Boolean = true):void {
			object.x = x;
			object.y = y;
			object.alpha = alpha;
			object.visible = visible;
			_frames.push(addChild(object));
		}

		protected function showframe (frame:int):void {
			const numFrames:int = _frames.length;
			if (numFrames < 1) return;
			else if (frame > numFrames) frame = numFrames;
			else if (frame < 1) frame = 1;
			_frame = frame--;
			for (var n:int = 0; n < numFrames; ++n) {
				_frames[n].visible = false;
			}
			_frames[frame].visible = true;
		}

		public override function get currentFrame ():int {
			return _frame
		}

		public override function get totalFrames ():int {
			return _frames.length;
		}
	}
}