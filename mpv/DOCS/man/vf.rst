VIDEO FILTERS
=============

Video filters allow you to modify the video stream and its properties. All of
the information described in this section applies to audio filters as well
(generally using the prefix ``--af`` instead of ``--vf``).

The exact syntax is:

``--vf=<filter1[=parameter1:parameter2:...],filter2,...>``
    Setup a chain of video filters. This consists on the filter name, and an
    option list of parameters after ``=``. The parameters are separated by
    ``:`` (not ``,``, as that starts a new filter entry).

    Before the filter name, a label can be specified with ``@name:``, where
    name is an arbitrary user-given name, which identifies the filter. This
    is only needed if you want to toggle the filter at runtime.

    A ``!`` before the filter name means the filter is disabled by default. It
    will be skipped on filter creation. This is also useful for runtime filter
    toggling.

    See the ``vf`` command (and ``toggle`` sub-command) for further explanations
    and examples.

    The general filter entry syntax is:

        ``["@"<label-name>":"] ["!"] <filter-name> [ "=" <filter-parameter-list> ]``

    or for the special "toggle" syntax (see ``vf`` command):

        ``"@"<label-name>``

    and the ``filter-parameter-list``:

        ``<filter-parameter> | <filter-parameter> "," <filter-parameter-list>``

    and ``filter-parameter``:

        ``( <param-name> "=" <param-value> ) | <param-value>``

    ``param-value`` can further be quoted in ``[`` / ``]`` in case the value
    contains characters like ``,`` or ``=``. This is used in particular with
    the ``lavfi`` filter, which uses a very similar syntax as mpv (MPlayer
    historically) to specify filters and their parameters.

Filters can be manipulated at run time. You can use ``@`` labels as described
above in combination with the ``vf`` command (see `COMMAND INTERFACE`_) to get
more control over this. Initially disabled filters with ``!`` are useful for
this as well.

You can also set defaults for each filter. The defaults are applied before the
normal filter parameters. This is deprecated and never worked for the
libavfilter bridge.

``--vf-defaults=<filter1[=parameter1:parameter2:...],filter2,...>``
    Set defaults for each filter. (Deprecated. ``--af-defaults`` is deprecated
    as well.)

.. note::

    To get a full list of available video filters, see ``--vf=help`` and
    http://ffmpeg.org/ffmpeg-filters.html .

    Also, keep in mind that most actual filters are available via the ``lavfi``
    wrapper, which gives you access to most of libavfilter's filters. This
    includes all filters that have been ported from MPlayer to libavfilter.

    Most builtin filters are deprecated in some ways, unless they're only available
    in mpv (such as filters which deal with mpv specifics, or which are
    implemented in mpv only).

    If a filter is not builtin, the ``lavfi-bridge`` will be automatically
    tried. This bridge does not support help output, and does not verify
    parameters before the filter is actually used. Although the mpv syntax
    is rather similar to libavfilter's, it's not the same. (Which means not
    everything accepted by vf_lavfi's ``graph`` option will be accepted by
    ``--vf``.)

    You can also prefix the filter name with ``lavfi-`` to force the wrapper.
    This is helpful if the filter name collides with a deprecated mpv builtin
    filter. For example ``--vf=lavfi-scale=args`` would use libavfilter's
    ``scale`` filter over mpv's deprecated builtin one.

Video filters are managed in lists. There are a few commands to manage the
filter list.

``--vf-add=filter``
    Appends the filter given as arguments to the filter list. (Passing multiple
    filters is currently still possible, but deprecated.)

``--vf-pre=filter``
    Prepends the filters given as arguments to the filter list. (Passing
    multiple filters is currently still possible, but deprecated.)

``--vf-del=filter``
    Deletes the filter. The filter can even given the way it was added (filter
    name and its full argument list), by label (prefixed with ``@``), or as
    index number. Index numbers start at 0, negative numbers address the end of
    the list (-1 is the last). (Passing multiple filters is currently still
    possible, but deprecated.)

``--vf-clr``
    Completely empties the filter list.

With filters that support it, you can access parameters by their name.

``--vf=<filter>=help``
    Prints the parameter names and parameter value ranges for a particular
    filter.

Available mpv-only filters are:

``format=fmt=<value>:colormatrix=<value>:...``
    Restricts the color space for the next filter without doing any conversion.
    Use together with the scale filter for a real conversion.

    .. note::

        For a list of available formats, see ``format=fmt=help``.

    ``<fmt>``
        Format name, e.g. rgb15, bgr24, 420p, etc. (default: don't change).

    ``<colormatrix>``
        Controls the YUV to RGB color space conversion when playing video. There
        are various standards. Normally, BT.601 should be used for SD video, and
        BT.709 for HD video. (This is done by default.) Using incorrect color space
        results in slightly under or over saturated and shifted colors.

        These options are not always supported. Different video outputs provide
        varying degrees of support. The ``gpu`` and ``vdpau`` video output
        drivers usually offer full support. The ``xv`` output can set the color
        space if the system video driver supports it, but not input and output
        levels. The ``scale`` video filter can configure color space and input
        levels, but only if the output format is RGB (if the video output driver
        supports RGB output, you can force this with ``-vf scale,format=rgba``).

        If this option is set to ``auto`` (which is the default), the video's
        color space flag will be used. If that flag is unset, the color space
        will be selected automatically. This is done using a simple heuristic that
        attempts to distinguish SD and HD video. If the video is larger than
        1279x576 pixels, BT.709 (HD) will be used; otherwise BT.601 (SD) is
        selected.

        Available color spaces are:

        :auto:          automatic selection (default)
        :bt.601:        ITU-R BT.601 (SD)
        :bt.709:        ITU-R BT.709 (HD)
        :bt.2020-ncl:   ITU-R BT.2020 non-constant luminance system
        :bt.2020-cl:    ITU-R BT.2020 constant luminance system
        :smpte-240m:    SMPTE-240M

    ``<colorlevels>``
        YUV color levels used with YUV to RGB conversion. This option is only
        necessary when playing broken files which do not follow standard color
        levels or which are flagged wrong. If the video does not specify its
        color range, it is assumed to be limited range.

        The same limitations as with ``<colormatrix>`` apply.

        Available color ranges are:

        :auto:      automatic selection (normally limited range) (default)
        :limited:   limited range (16-235 for luma, 16-240 for chroma)
        :full:      full range (0-255 for both luma and chroma)

    ``<primaries>``
        RGB primaries the source file was encoded with. Normally this should be set
        in the file header, but when playing broken or mistagged files this can be
        used to override the setting.

        This option only affects video output drivers that perform color
        management, for example ``gpu`` with the ``target-prim`` or
        ``icc-profile`` suboptions set.

        If this option is set to ``auto`` (which is the default), the video's
        primaries flag will be used. If that flag is unset, the color space will
        be selected automatically, using the following heuristics: If the
        ``<colormatrix>`` is set or determined as BT.2020 or BT.709, the
        corresponding primaries are used. Otherwise, if the video height is
        exactly 576 (PAL), BT.601-625 is used. If it's exactly 480 or 486 (NTSC),
        BT.601-525 is used. If the video resolution is anything else, BT.709 is
        used.

        Available primaries are:

        :auto:         automatic selection (default)
        :bt.601-525:   ITU-R BT.601 (SD) 525-line systems (NTSC, SMPTE-C)
        :bt.601-625:   ITU-R BT.601 (SD) 625-line systems (PAL, SECAM)
        :bt.709:       ITU-R BT.709 (HD) (same primaries as sRGB)
        :bt.2020:      ITU-R BT.2020 (UHD)
        :apple:        Apple RGB
        :adobe:        Adobe RGB (1998)
        :prophoto:     ProPhoto RGB (ROMM)
        :cie1931:      CIE 1931 RGB
        :dci-p3:       DCI-P3 (Digital Cinema)
        :v-gamut:      Panasonic V-Gamut primaries

    ``<gamma>``
       Gamma function the source file was encoded with. Normally this should be set
       in the file header, but when playing broken or mistagged files this can be
       used to override the setting.

       This option only affects video output drivers that perform color management.

       If this option is set to ``auto`` (which is the default), the gamma will
       be set to BT.1886 for YCbCr content, sRGB for RGB content and Linear for
       XYZ content.

       Available gamma functions are:

       :auto:         automatic selection (default)
       :bt.1886:      ITU-R BT.1886 (EOTF corresponding to BT.601/BT.709/BT.2020)
       :srgb:         IEC 61966-2-4 (sRGB)
       :linear:       Linear light
       :gamma1.8:     Pure power curve (gamma 1.8)
       :gamma2.2:     Pure power curve (gamma 2.2)
       :gamma2.8:     Pure power curve (gamma 2.8)
       :prophoto:     ProPhoto RGB (ROMM) curve
       :pq:           ITU-R BT.2100 PQ (Perceptual quantizer) curve
       :hlg:          ITU-R BT.2100 HLG (Hybrid Log-gamma) curve
       :v-log:        Panasonic V-Log transfer curve
       :s-log1:       Sony S-Log1 transfer curve
       :s-log2:       Sony S-Log2 transfer curve

    ``<sig-peak>``
        Reference peak illumination for the video file, relative to the
        signal's reference white level. This is mostly interesting for HDR, but
        it can also be used tone map SDR content to simulate a different
        exposure. Normally inferred from tags such as MaxCLL or mastering
        metadata.

        The default of 0.0 will default to the source's nominal peak luminance.

    ``<light>``
        Light type of the scene. This is mostly correctly inferred based on the
        gamma function, but it can be useful to override this when viewing raw
        camera footage (e.g. V-Log), which is normally scene-referred instead
        of display-referred.

        Available light types are:

       :auto:         Automatic selection (default)
       :display:      Display-referred light (most content)
       :hlg:          Scene-referred using the HLG OOTF (e.g. HLG content)
       :709-1886:     Scene-referred using the BT709+BT1886 interaction
       :gamma1.2:     Scene-referred using a pure power OOTF (gamma=1.2)

    ``<stereo-in>``
        Set the stereo mode the video is assumed to be encoded in. Takes the
        same values as the ``--video-stereo-mode`` option.

    ``<stereo-out>``
        Set the stereo mode the video should be displayed as. Takes the
        same values as the ``--video-stereo-mode`` option.

    ``<rotate>``
        Set the rotation the video is assumed to be encoded with in degrees.
        The special value ``-1`` uses the input format.

    ``<dw>``, ``<dh>``
        Set the display size. Note that setting the display size such that
        the video is scaled in both directions instead of just changing the
        aspect ratio is an implementation detail, and might change later.

    ``<dar>``
        Set the display aspect ratio of the video frame. This is a float,
        but values such as ``[16:9]`` can be passed too (``[...]`` for quoting
        to prevent the option parser from interpreting the ``:`` character).

    ``<spherical-type>``
        Type of the spherical projection:

        :auto:      As indicated by the file (default)
        :none:      Normal video
        :equirect:  Equirectangular
        :unknown:   Unknown projection

    ``<spherical-yaw>``, ``<spherical-pitch>``, ``<spherical-roll>``
        Reference angle in degree, if spherical video is used.

``lavfi=graph[:sws-flags[:o=opts]]``
    Filter video using FFmpeg's libavfilter.

    ``<graph>``
        The libavfilter graph string. The filter must have a single video input
        pad and a single video output pad.

        See `<https://ffmpeg.org/ffmpeg-filters.html>`_ for syntax and available
        filters.

        .. warning::

            If you want to use the full filter syntax with this option, you have
            to quote the filter graph in order to prevent mpv's syntax and the
            filter graph syntax from clashing. To prevent a quoting and escaping
            mess, consider using ``--lavfi-complex`` if you know which video
            track you want to use from the input file. (There is only one video
            track for nearly all video files anyway.)

        .. admonition:: Examples

            ``--vf=lavfi=[gradfun=20:30,vflip]``
                ``gradfun`` filter with nonsense parameters, followed by a
                ``vflip`` filter. (This demonstrates how libavfilter takes a
                graph and not just a single filter.) The filter graph string is
                quoted with ``[`` and ``]``. This requires no additional quoting
                or escaping with some shells (like bash), while others (like
                zsh) require additional ``"`` quotes around the option string.

            ``'--vf=lavfi="gradfun=20:30,vflip"'``
                Same as before, but uses quoting that should be safe with all
                shells. The outer ``'`` quotes make sure that the shell does not
                remove the ``"`` quotes needed by mpv.

            ``'--vf=lavfi=graph="gradfun=radius=30:strength=20,vflip"'``
                Same as before, but uses named parameters for everything.

    ``<sws-flags>``
        If libavfilter inserts filters for pixel format conversion, this
        option gives the flags which should be passed to libswscale. This
        option is numeric and takes a bit-wise combination of ``SWS_`` flags.

        See ``http://git.videolan.org/?p=ffmpeg.git;a=blob;f=libswscale/swscale.h``.

    ``<o>``
        Set AVFilterGraph options. These should be documented by FFmpeg.

        .. admonition:: Example

            ``'--vf=lavfi=yadif:o="threads=2,thread_type=slice"'``
                forces a specific threading configuration.

``sub=[=bottom-margin:top-margin]``
    Moves subtitle rendering to an arbitrary point in the filter chain, or force
    subtitle rendering in the video filter as opposed to using video output OSD
    support.

    ``<bottom-margin>``
        Adds a black band at the bottom of the frame. The SSA/ASS renderer can
        place subtitles there (with ``--sub-use-margins``).
    ``<top-margin>``
        Black band on the top for toptitles  (with ``--sub-use-margins``).

    .. admonition:: Examples

        ``--vf=sub,eq``
            Moves sub rendering before the eq filter. This will put both
            subtitle colors and video under the influence of the video equalizer
            settings.

``vapoursynth=file:buffered-frames:concurrent-frames``
    Loads a VapourSynth filter script. This is intended for streamed
    processing: mpv actually provides a source filter, instead of using a
    native VapourSynth video source. The mpv source will answer frame
    requests only within a small window of frames (the size of this window
    is controlled with the ``buffered-frames`` parameter), and requests outside
    of that will return errors. As such, you can't use the full power of
    VapourSynth, but you can use certain filters.

    If you just want to play video generated by a VapourSynth (i.e. using
    a native VapourSynth video source), it's better to use ``vspipe`` and a
    FIFO to feed the video to mpv. The same applies if the filter script
    requires random frame access (see ``buffered-frames`` parameter).

    This filter is experimental. If it turns out that it works well and is
    used, it will be ported to libavfilter. Otherwise, it will be just removed.

    ``file``
        Filename of the script source. Currently, this is always a python
        script. The variable ``video_in`` is set to the mpv video source,
        and it is expected that the script reads video from it. (Otherwise,
        mpv will decode no video, and the video packet queue will overflow,
        eventually leading to audio being stopped.) The script is also
        expected to pass through timestamps using the ``_DurationNum`` and
        ``_DurationDen`` frame properties.

        .. admonition:: Example:

            ::

                import vapoursynth as vs
                core = vs.get_core()
                core.std.AddBorders(video_in, 10, 10, 20, 20).set_output()

        .. warning::

            The script will be reloaded on every seek. This is done to reset
            the filter properly on discontinuities.

    ``buffered-frames``
        Maximum number of decoded video frames that should be buffered before
        the filter (default: 4). This specifies the maximum number of frames
        the script can request in reverse direction.
        E.g. if ``buffered-frames=5``, and the script just requested frame 15,
        it can still request frame 10, but frame 9 is not available anymore.
        If it requests frame 30, mpv will decode 15 more frames, and keep only
        frames 25-30.

        The actual number of buffered frames also depends on the value of the
        ``concurrent-frames`` option. Currently, both option values are
        multiplied to get the final buffer size.

        (Normally, VapourSynth source filters must provide random access, but
        mpv was made for playback, and does not provide frame-exact random
        access. The way this video filter works is a compromise to make simple
        filters work anyway.)

    ``concurrent-frames``
        Number of frames that should be requested in parallel. The
        level of concurrency depends on the filter and how quickly mpv can
        decode video to feed the filter. This value should probably be
        proportional to the number of cores on your machine. Most time,
        making it higher than the number of cores can actually make it
        slower.

        By default, this uses the special value ``auto``, which sets the option
        to the number of detected logical CPU cores.

    The following variables are defined by mpv:

    ``video_in``
        The mpv video source as vapoursynth clip. Note that this has no length
        set, which confuses many filters. Using ``Trim`` on the clip with a
        high dummy length can turn it into a finite clip.

    ``video_in_dw``, ``video_in_dh``
        Display size of the video. Can be different from video size if the
        video does not use square pixels (e.g. DVD).

    ``container_fps``
        FPS value as reported by file headers. This value can be wrong or
        completely broken (e.g. 0 or NaN). Even if the value is correct,
        if another filter changes the real FPS (by dropping or inserting
        frames), the value of this variable might not be useful. Note that
        the ``--fps`` command line option overrides this value.

        Useful for some filters which insist on having a FPS.

    ``display_fps``
        Refresh rate of the current display. Note that this value can be 0.

``vapoursynth-lazy``
    The same as ``vapoursynth``, but doesn't load Python scripts. Instead, a
    custom backend using Lua and the raw VapourSynth API is used. The syntax
    is completely different, and absolutely no convenience features are
    provided. There's no type checking either, and you can trigger crashes.

    .. admonition:: Example:

        ::

            video_out = invoke("morpho", "Open", {clip = video_in})

    The special variable ``video_in`` is the mpv video source, while the
    special variable ``video_out`` is used to read video from. The 1st argument
    is the plugin (queried with ``getPluginByNs``), the 2nd is the filter name,
    and the 3rd argument is a table with the arguments. Positional arguments
    are not supported. The types must match exactly. Since Lua is terrible and
    can't distinguish integers and floats, integer arguments must be prefixed
    with ``i_``, in which case the prefix is removed and the argument is cast
    to an integer. Should the argument's name start with ``i_``, you're out of
    luck.

    Clips (VSNodeRef) are passed as light userdata, so trying to pass any
    other userdata type will result in hard crashes.

``vavpp``
    VA-AP-API video post processing. Works with ``--vo=vaapi`` and ``--vo=gpu``
    only. Currently deinterlaces. This filter is automatically inserted if
    deinterlacing is requested (either using the ``d`` key, by default mapped to
    the command ``cycle deinterlace``, or the ``--deinterlace`` option).

    ``deint=<method>``
        Select the deinterlacing algorithm.

        no
            Don't perform deinterlacing.
        auto
             Select the best quality deinterlacing algorithm (default). This
             goes by the order of the options as documented, with
             ``motion-compensated`` being considered best quality.
        first-field
            Show only first field.
        bob
            bob deinterlacing.
        weave, motion-adaptive, motion-compensated
            Advanced deinterlacing algorithms. Whether these actually work
            depends on the GPU hardware, the GPU drivers, driver bugs, and
            mpv bugs.

    ``<interlaced-only>``
        :no:  Deinterlace all frames (default).
        :yes: Only deinterlace frames marked as interlaced.

    ``reversal-bug=<yes|no>``
        :no:  Use the API as it was interpreted by older Mesa drivers. While
              this interpretation was more obvious and inuitive, it was
              apparently wrong, and not shared by Intel driver developers.
        :yes: Use Intel interpretation of surface forward and backwards
              references (default). This is what Intel drivers and newer Mesa
              drivers expect. Matters only for the advanced deinterlacing
              algorithms.

``vdpaupp``
    VDPAU video post processing. Works with ``--vo=vdpau`` and ``--vo=gpu``
    only. This filter is automatically inserted if deinterlacing is requested
    (either using the ``d`` key, by default mapped to the command
    ``cycle deinterlace``, or the ``--deinterlace`` option). When enabling
    deinterlacing, it is always preferred over software deinterlacer filters
    if the ``vdpau`` VO is used, and also if ``gpu`` is used and hardware
    decoding was activated at least once (i.e. vdpau was loaded).

    ``sharpen=<-1-1>``
        For positive values, apply a sharpening algorithm to the video, for
        negative values a blurring algorithm (default: 0).
    ``denoise=<0-1>``
        Apply a noise reduction algorithm to the video (default: 0; no noise
        reduction).
    ``deint=<yes|no>``
        Whether deinterlacing is enabled (default: no). If enabled, it will use
        the mode selected with ``deint-mode``.
    ``deint-mode=<first-field|bob|temporal|temporal-spatial>``
        Select deinterlacing mode (default: temporal).

        Note that there's currently a mechanism that allows the ``vdpau`` VO to
        change the ``deint-mode`` of auto-inserted ``vdpaupp`` filters. To avoid
        confusion, it's recommended not to use the ``--vo=vdpau`` suboptions
        related to filtering.

        first-field
            Show only first field.
        bob
            Bob deinterlacing.
        temporal
            Motion-adaptive temporal deinterlacing. May lead to A/V desync
            with slow video hardware and/or high resolution.
        temporal-spatial
            Motion-adaptive temporal deinterlacing with edge-guided spatial
            interpolation. Needs fast video hardware.
    ``chroma-deint``
        Makes temporal deinterlacers operate both on luma and chroma (default).
        Use no-chroma-deint to solely use luma and speed up advanced
        deinterlacing. Useful with slow video memory.
    ``pullup``
        Try to apply inverse telecine, needs motion adaptive temporal
        deinterlacing.
    ``interlaced-only=<yes|no>``
        If ``yes``, only deinterlace frames marked as interlaced (default: no).
    ``hqscaling=<0-9>``
        0
            Use default VDPAU scaling (default).
        1-9
            Apply high quality VDPAU scaling (needs capable hardware).

``d3d11vpp``
    Direct3D 11 video post processing. Currently requires D3D11 hardware
    decoding for use.

    ``deint=<yes|no>``
        Whether deinterlacing is enabled (default: no).
    ``interlaced-only=<yes|no>``
        If ``yes``, only deinterlace frames marked as interlaced (default: no).
    ``mode=<blend|bob|adaptive|mocomp|ivctc|none>``
        Tries to select a video processor with the given processing capability.
        If a video processor supports multiple capabilities, it is not clear
        which algorithm is actually selected. ``none`` always falls back. On
        most if not all hardware, this option will probably do nothing, because
        a video processor usually supports all modes or none.
