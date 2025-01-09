from JDiffusion import StableDiffusionPipeline
import jittor as jt
jt.flags.use_cuda = 1


pipe = StableDiffusionPipeline.from_pretrained("stabilityai/stable-diffusion-2-1",dtype=jt.float32,safety_checker=None)

text = ["a photo of cute dog"]

images = pipe(text*2, height=512, width=512,seed=[20,30]).images

images[0].save('./stable_diffusion_output_2.jpg')
images[1].save('./stable_diffusion_output_3.jpg')
